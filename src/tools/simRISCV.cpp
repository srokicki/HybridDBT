/*
 * simRISCV.cpp
 *
 *  Created on: 5 déc. 2016
 *      Author: Simon Rokicki
 */


#include <simulator/riscvSimulator.h>

#include <cstdio>
#include <cstdlib>
#include <lib/elfFile.h>

//Main function performing the merging
int main(int argc, char* charv[]){

	//Parsing of function arguments. The function is called with the elf file to translate
	if (argc != 2){
		printf("Error: incorrect number of arguments.\n Usage of the function is ./simRISCV elfFile\n");
		exit(-1);
	}

	//******************************************************************************************
	//Opening elf files
	ElfFile elfFile(charv[1]);
	RiscvSimulator* simulator = new RiscvSimulator();


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

	simulator->doSimulation(0x10000);

}
