#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "header.h"
#include <locale.h>

int seek_s = 0;
#ifndef SEEK_END
#define SEEK_END 2
#endif

long file_size (char *name)
{
	long eof_ftell;
	FILE *file;

	file = fopen(name, "r");
	if (file == NULL)
		return(0L);
	fseek(file, 0L, SEEK_END);
	eof_ftell = ftell(file);
	fclose(file);
	return(eof_ftell);
}



void print_ratios (char *input, char *output)
{
	long input_size;
	long output_size;
	int ratio;

	input_size = file_size(input);
	if (input_size == 0)
		input_size = 1;
	output_size = file_size(output);
	ratio = 100 - (int) (output_size * 100L / input_size);
	printf("\nРазмер сжатия: %ld\n", input_size);
	printf("ђ §¬Ґа б¦ в(r)Ј(r) д (c)« :\t%ld\n", output_size);
	if (output_size == 0)
		output_size = 1;
	printf("Степень сжатие:%d%\n", ratio);
}



void fatal_error (char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	printf("Ошибка : ");
	vprintf(fmt, argptr);
	va_end(argptr);
	exit(-1);
}


#define PACIFIER_COUNT 2047




COMPRESSED_FILE *OpenOutputCompressedFile (char *name)
{
	COMPRESSED_FILE *compressed_file;

	compressed_file = (COMPRESSED_FILE *) calloc(1, sizeof(COMPRESSED_FILE));
	if (compressed_file == NULL)
		return(compressed_file);
	compressed_file->file = fopen(name, "wb");
	compressed_file->rack = 0;
	compressed_file->mask = 0x80;
	compressed_file->pacifier_counter = 0;
	return(compressed_file);
}



COMPRESSED_FILE *OpenInputCompressedFile (char *name)
{
	COMPRESSED_FILE *compressed_file;

	compressed_file = (COMPRESSED_FILE *) calloc(1, sizeof(COMPRESSED_FILE));
	if (compressed_file == NULL)
		return(compressed_file);
	compressed_file->file = fopen(name, "rb");
	compressed_file->rack = 0;
	compressed_file->mask = 0x80;
	compressed_file->pacifier_counter = 0;
	return(compressed_file);
}


void CloseOutputCompressedFile(COMPRESSED_FILE *compressed_file)
{
	if (compressed_file->mask != 0x80)
		if (putc(compressed_file->rack, compressed_file->file) != compressed_file->rack)
			fatal_error("Ошибки при попытки закртыть сжатый файл \n");
	fclose(compressed_file->file);
	free((char *) compressed_file);
}



void CloseInputCompressedFile(COMPRESSED_FILE *compressed_file)
{
	fclose(compressed_file->file);
	free((char *) compressed_file);
}



void OutputBit(COMPRESSED_FILE *compressed_file, int bit)
{
	if (bit)
		compressed_file->rack |= compressed_file->mask;
	compressed_file->mask >>= 1;
	if (compressed_file->mask == 0)
	{
		if (putc(compressed_file->rack, compressed_file->file) != compressed_file->rack)
			fatal_error("Ошибки в процедуре OutputBit!\n");
		else if ((compressed_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
			putc('.', stdout);
		compressed_file->rack = 0;
		compressed_file->mask = 0x80;
	}
}



void OutputBits(COMPRESSED_FILE *compressed_file, unsigned long code, int count)
{
	unsigned long mask;

	mask = 1L << (count - 1);
	while (mask != 0)
	{
		if (mask & code)
			compressed_file->rack |= compressed_file->mask;
		compressed_file->mask >>= 1;
		if (compressed_file->mask == 0)
		{
			if (putc(compressed_file->rack, compressed_file->file) != compressed_file->rack)
				fatal_error("Ошибка в процедуре OutputBits!\n");
			else if ((compressed_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
				putc('.', stdout);
			compressed_file->rack = 0;
			compressed_file->mask = 0x80;
		}
		mask >>= 1;
	}
}



int InputBit (COMPRESSED_FILE *compressed_file)
{
	int value;

	if (compressed_file->mask == 0x80)
	{
		compressed_file->rack = getc(compressed_file->file);
		if (compressed_file->rack == EOF)
			fatal_error("Ошибка в процедуре InputBit!\n");
		if ((compressed_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
			putc('.', stdout);
	}
	value = compressed_file->rack & compressed_file->mask;
	compressed_file->mask >>= 1;
	if (compressed_file->mask == 0)
		compressed_file->mask = 0x80;
	return(value ? 1 : 0);
}


unsigned long InputBits (COMPRESSED_FILE *compressed_file, int bit_count)
{
	unsigned long mask;
	unsigned long return_value;

	mask = 1L << (bit_count - 1);
	return_value = 0;
	while (mask != 0)
	{
		if (compressed_file->mask == 0x80)
		{
			compressed_file->rack = getc(compressed_file->file);
			if (compressed_file->rack == EOF)
				fatal_error("Ошибка в процедуре InputBits!\n");
			if ((compressed_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
				putc('.', stdout);
		}
		if (compressed_file->rack & compressed_file->mask)
			return_value |= mask;
		mask >>= 1;
		compressed_file->mask >>= 1;
		if (compressed_file->mask == 0)
			compressed_file->mask = 0x80;
	}
	return (return_value);
}



char *CompressionName = "Кодирование";
char *Usage = "Что сжимать - куда сжимать";


TREE Tree;



void CompressFile(char **filenames, int lenght, COMPRESSED_FILE *output)
{
    int len[lenght];
    int i;
	InitializeTree(&Tree);
	int c;
	for(i = 0; i < lenght; i++) {
        len[i] = 0;
        FILE *input  = fopen(filenames[i], "rb");
        if (input == NULL)
            fatal_error("Не удалось найти файл input: %s", filenames[i]);
        while ((c = getc(input)) != EOF)
        {
            len[i]++;
        }
        fclose(input);
	}

	fprintf(output->file,"\n\\%d\\",lenght);
    fputs("filename", output->file);
    for(i = 0; i < lenght; i++){
            fputs("\\", output->file);
            fputs(filenames[i], output->file);
            fprintf(output->file,"\\%d",len[i]);
    }
    fputs("\n", output->file);
    for(i = 0; i < lenght; i++) {
        FILE *input  = fopen(filenames[i], "rb");
        while ((c = getc(input)) != EOF)
        {
            EncodeSymbol(&Tree, c, output);
            UpdateModel(&Tree, c);
        }
        fclose(input);
	}
    EncodeSymbol(&Tree, END_OF_STREAM, output);
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 2);
    strcpy(result, s1);
    strcat(result, "\\");
    strcat(result, s2);
    return result;
}

void ExpandFile (COMPRESSED_FILE *input, char* directory)
{

    int length;
    int i,s2;

    fscanf(input->file, "\n\\%d\\filename",&length);
    char names[256];
    fscanf(input->file, "\\%s",names);
    char *files[length];
    int len[length];
    char *temp = strtok(names, "\\");
    char file[256] = " ";
    for(i = 0; i < length; i++)
    {
        files[i] = temp;
        temp = strtok (NULL,"\\");
        len[i] = atoi(temp);
        temp = strtok (NULL,"\\");
    }

    fseek(input->file,1, SEEK_CUR);

    InitializeTree(&Tree);
    for(i = 0; i < length; i++){

        char *s = concat(directory,files[i]);
        int c;
        char s1;
        FILE *output = fopen(s,"wb");
        if(output == NULL) {
            return;
        }
        while (len[i] != 0)
        {
            c = DecodeSymbol(&Tree, input);
            if (putc(c, output) == EOF)
                fatal_error("Не вышло распаковать файл!");
            UpdateModel(&Tree, c);
            len[i]--;
        }

    }
}


void InitializeTree (TREE *tree)
{
	int i;

	tree->nodes[ ROOT_NODE ].child			 = ROOT_NODE + 1;
	tree->nodes[ ROOT_NODE ].child_is_leaf	 = FALSE;
	tree->nodes[ ROOT_NODE ].weight			= 2;
	tree->nodes[ ROOT_NODE ].parent			= -1;

	tree->nodes[ ROOT_NODE + 1 ].child		 = END_OF_STREAM;
	tree->nodes[ ROOT_NODE + 1 ].child_is_leaf = TRUE;
	tree->nodes[ ROOT_NODE + 1 ].weight		= 1;
	tree->nodes[ ROOT_NODE + 1 ].parent		= ROOT_NODE;
	tree->leaf[ END_OF_STREAM ]				= ROOT_NODE + 1;

	tree->nodes[ ROOT_NODE + 2 ].child		 = ESCAPE;
	tree->nodes[ ROOT_NODE + 2 ].child_is_leaf = TRUE;
	tree->nodes[ ROOT_NODE + 2 ].weight		= 1;
	tree->nodes[ ROOT_NODE + 2 ].parent		= ROOT_NODE;
	tree->leaf[ ESCAPE ]					   = ROOT_NODE + 2;

	tree->next_free_node					   = ROOT_NODE + 3;

	for (i = 0; i < END_OF_STREAM; i++)
		tree->leaf[ i ] = -1;
}



void EncodeSymbol (TREE *tree, unsigned int c, COMPRESSED_FILE *output)
{
	unsigned long code;
	unsigned long current_bit;
	int code_size;
	int current_node;

	code = 0;
	current_bit = 1;
	code_size = 0;
	current_node = tree->leaf[ c ];
	if (current_node == -1)
		current_node = tree->leaf[ ESCAPE ];
	while (current_node != ROOT_NODE)
	{
		if ((current_node & 1) == 0)
			code |= current_bit;
		current_bit <<= 1;
		code_size++;
		current_node = tree->nodes[ current_node ].parent;
	}
	OutputBits(output, code, code_size);
	if (tree->leaf[ c ] == -1)
	{
		OutputBits(output, (unsigned long) c, 8);
		add_new_node(tree, c);
	}
}

int DecodeSymbol (TREE *tree, COMPRESSED_FILE *input)
{
	int current_node;
	int c;

	current_node = ROOT_NODE;
	while (!tree->nodes[ current_node ].child_is_leaf)
	{
		current_node = tree->nodes[ current_node ].child;
		current_node += InputBit(input);
	}
	c = tree->nodes[ current_node ].child;
	if (c == ESCAPE)
	{
		c = (int) InputBits(input, 8);
		add_new_node(tree, c);
	}
	return(c);
}


void UpdateModel (TREE *tree, int c)
{
	int current_node;
	int new_node;

	if (tree->nodes[ ROOT_NODE].weight == MAX_WEIGHT)
		RebuildTree(tree);
	current_node = tree->leaf[ c ];
	while (current_node != -1)
	{
		tree->nodes[ current_node ].weight++;
		for (new_node = current_node; new_node > ROOT_NODE; new_node--)
			if (tree->nodes[ new_node - 1 ].weight >=
					tree->nodes[ current_node ].weight)
				break;
		if (current_node != new_node)
		{
			swap_nodes(tree, current_node, new_node);
			current_node = new_node;
		}
		current_node = tree->nodes[ current_node ].parent;
	}
}


void RebuildTree (TREE *tree)
{
	int i;
	int j;
	int k;
	unsigned int weight;

	printf("R");
	j = tree->next_free_node - 1;
	for (i = j; i >= ROOT_NODE; i--)
	{
		if (tree->nodes[ i ].child_is_leaf)
		{
			tree->nodes[ j ] = tree->nodes[ i ];
			tree->nodes[ j ].weight = (tree->nodes[ j ].weight + 1) / 2;
			j--;
		}
	}

	for (i = tree->next_free_node - 2; j >= ROOT_NODE; i -= 2, j--)
	{
		k = i + 1;
		tree->nodes[ j ].weight = tree->nodes[ i ].weight + tree->nodes[ k ].weight;
		weight = tree->nodes[ j ].weight;
		tree->nodes[ j ].child_is_leaf = FALSE;
		for (k = j + 1; weight < tree->nodes[ k ].weight; k++)
		;
		k--;
		memmove(&tree->nodes[ j ], &tree->nodes[ j + 1 ],
			(k - j) * sizeof(struct node));
		tree->nodes[ k ].weight = weight;
		tree->nodes[ k ].child = i;
		tree->nodes[ k ].child_is_leaf = FALSE;
	}

	for (i = tree->next_free_node - 1; i >= ROOT_NODE; i--)
	{
		if (tree->nodes[ i ].child_is_leaf)
		{
			k = tree->nodes[ i ].child;
			tree->leaf[ k ] = i;
		}
		else
		{
			k = tree->nodes[ i ].child;
			tree->nodes[ k ].parent = tree->nodes[ k + 1 ].parent = i;
		}
	}
}



void swap_nodes (TREE *tree, int i, int j)
{
	struct node temp;

	if (tree->nodes[ i ].child_is_leaf)
		tree->leaf[ tree->nodes[ i ].child ] = j;
	else
	{
		tree->nodes[ tree->nodes[ i ].child ].parent = j;
		tree->nodes[ tree->nodes[ i ].child + 1 ].parent = j;
	}
	if (tree->nodes[ j ].child_is_leaf)
		tree->leaf[ tree->nodes[ j ].child ] = i;
	else
	{
		tree->nodes[ tree->nodes[ j ].child ].parent = i;
		tree->nodes[ tree->nodes[ j ].child + 1 ].parent = i;
	}
	temp = tree->nodes[ i ];
	tree->nodes[ i ] = tree->nodes[ j ];
	tree->nodes[ i ].parent = temp.parent;
	temp.parent = tree->nodes[ j ].parent;
	tree->nodes[ j ] = temp;
}

void add_new_node (TREE *tree, int c)
{
	int lightest_node;
	int new_node;
	int zero_weight_node;

	lightest_node = tree->next_free_node - 1;
	new_node = tree->next_free_node;
	zero_weight_node = tree->next_free_node + 1;
	tree->next_free_node += 2;

	tree->nodes[ new_node ] = tree->nodes[ lightest_node ];
	tree->nodes[ new_node ].parent = lightest_node;
	tree->leaf[ tree->nodes[ new_node ].child ] = new_node;

	tree->nodes[ lightest_node ].child		 = new_node;
	tree->nodes[ lightest_node ].child_is_leaf = FALSE;

	tree->nodes[ zero_weight_node ].child		   = c;
	tree->nodes[ zero_weight_node ].child_is_leaf   = TRUE;
	tree->nodes[ zero_weight_node ].weight		  = 0;
	tree->nodes[ zero_weight_node ].parent		  = lightest_node;
	tree->leaf[ c ] = zero_weight_node;
}

int main (int argc, char *argv[])
{

    setlocale( LC_ALL, "Russian" );

	setbuf(stdout, NULL);


    COMPRESSED_FILE *output;
    COMPRESSED_FILE *compInput;
    FILE *input;
    char* directory;
    int length;
    char names[256];
    int i;
    char *temp = strtok(names, "\\");
    char file[256] = " ";
    if(strcmp(argv[3], "--extract") == 0){
        compInput = OpenInputCompressedFile(argv[2]);
        ExpandFile(compInput,argv[4]);
        CloseInputCompressedFile(compInput);

    } else if(strcmp(argv[3],"--create") == 0) {
        output = OpenOutputCompressedFile(argv[2]);
        if (output == NULL)
            fatal_error("Не удалось создать файл output: %s\n", argv[1]);
            char *names[argc - 3];
        for(int i = 4; i < argc - 3 + 4; i++) {
            names[i-4]=argv[i];
        }
        CompressFile(names,argc - 4, output);
        CloseOutputCompressedFile(output);
    } else if(strcmp(argv[3],"--list") == 0 && argc < 4) {
        compInput = OpenInputCompressedFile(argv[2]);
        fscanf(compInput->file, "\n\\%d\\filename",&length);
        fscanf(compInput->file, "\\%s",names);
        for(i = 0; i < length; i++)
        {
            temp = strtok (NULL,"\\");
            printf("Файл %d - %s\n",i+1,temp);
            temp = strtok (NULL,"\\");
        }

    } else {
        printf("\n1)Создание архива: <имя EXE файла> --files <Входной файл> --create <Файлы>\n");
        printf("\n2)Экспорт из архива: <имя EXE файла> --files <Входной файл> --extract <Путь для экспорта>\n");
        printf("\n3)Список файлов в архиве: <имя EXE файла> --files <Входной файл> --list\n");
    }

    exit(0);
	return(0);
}