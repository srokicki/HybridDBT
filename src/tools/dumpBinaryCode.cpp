#include <stdio.h>

#include <lib/elfFile.h>
#include <lib/tools.h>
#include <lib/endianness.h>


int main(int argc, char *argv[])
{
	printf("/*Extraction of code from %s*/ \n", argv[1]);

	ElfFile elfFile((char *) argv[1]);

	unsigned char* code;
	unsigned int addressStart;
	unsigned int size;

	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);

		if (!section->getName().compare(".text")){

			code = section->getSectionCode();
			addressStart = section->address;
			size = section->size/4;

			printf("unsigned int size = %d;\n", size);
			printf("unsigned int addressStart = %d;\n", addressStart);

			printf("unsigned char code[%d] = {\n", size*4 + 1);
			for (int line = 0; line < size/4; line++){
				printf("\t");
				for (int column = 0; column<16; column++){
					printf("0x%x, ", code[16*line + column]);
				}
				printf("\n");
			}

			printf("0};\n");

		}
	}
}
