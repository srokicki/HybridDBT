#include <simulator/mipsSimulator.h>

#include <cstdio>
#include <cstdlib>
#include <lib/elfFile.h>

//Main function performing the merging
int main(int argc, char* charv[]){

	//Parsing of function arguments. The function is called with the elf file to translate
	if (argc != 2){
		printf("Error: incorrect number of arguments.\n Usage of the function is ./sim elfFile\n");
		exit(-1);
	}

	//******************************************************************************************
	//Opening elf files
	ElfFile elfFile(charv[1]);
	Simulator* simulator = new Simulator();


	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *oneSection = elfFile.sectionTable->at(sectionCounter);

		if (oneSection->address != 0){
			//If the address is not null we place its content into memory
			unsigned char* sectionContent = oneSection->getSectionCode();

			for (unsigned int byteNumber = 0; byteNumber<oneSection->size; byteNumber++){
				simulator->stb(oneSection->address + byteNumber, sectionContent[byteNumber]);
			}
		}
	}

	simulator->doSimulation(0xa002003c);

}
