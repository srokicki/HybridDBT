#include <stdio.h>

#include <lib/tools.h>
#include <lib/endianness.h>
#include <lib/elfFile.h>


int main(int argc, char *argv[])
{
	printf("/*Extraction of code from %s to %s*/ \n", argv[1], argv[2]);

	FILE* bytecodeFile = fopen(argv[1], "r");
	FILE* destinationFile = fopen(argv[2], "w");

	int size = 0;

	unsigned int result = 1;
	while (result == 1){
		char oneChar;
		result = fread(&oneChar, 1, 1, bytecodeFile);
		size++;
	}
	fclose(bytecodeFile);

	bytecodeFile = fopen(argv[1], "r");

	fprintf(destinationFile, "unsigned char bytecode[%d] = {\n", size+1);

	size = 0;
	result = 1;
	while (result == 1){
		unsigned char oneChar;
		result = fread(&oneChar, 1, 1, bytecodeFile);
		fprintf(destinationFile, "0x%x, ", oneChar);
		size++;
		if (size == 20){
			fprintf(destinationFile, "\n");
			size=0;
		}
	}

	fprintf(destinationFile, "0};\n");
	fclose(bytecodeFile);
}
