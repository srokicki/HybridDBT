/*
 * simRISCV.cpp
 *
 *  Created on: 5 déc. 2016
 *      Author: Simon Rokicki
 */


#include <simulator/riscvSimulator.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <lib/elfFile.h>
#include <unistd.h>

//Main function performing the merging
int main(int argc, char* argv[]){

	/*Parsing arguments of the comment
	 *
	 */

	int c;
	int VERBOSE = 0;
	int HELP = 0;
	char* FILE = NULL;
	char* ARGUMENTS = NULL;
	printf("%s\n", argv[3]);

	while ((c = getopt (argc, argv, "vha:")) != -1)
	switch (c)
	  {
	  case 'v':
		VERBOSE = 1;
		break;
	  case 'h':
		HELP = 1;
		break;
	  case 'a':
		  ARGUMENTS = optarg;
		break;
	  default:
		abort ();
	  }

	printf("%s\n", ARGUMENTS);
	if (optind < argc){
		FILE = argv[optind];
	}

	int localArgc;
	char** localArgv;
	if (ARGUMENTS == NULL){
		localArgc = argc - optind;
		localArgv =  &(argv[optind]);
	}
	else{
		//We find the size of the string representing arguments and replace spaces by 0
		int index = 0;
		int count = 1;
		while (ARGUMENTS[index] != 0){
			if (ARGUMENTS[index] == ' '){
				ARGUMENTS[index] = 0;
				count++;
				printf("Arg was %s\n", ARGUMENTS);
			}
			index++;
		}
		index++; //so that index is equal to the size of the char*


		//We find size of filename
		int charFileIndex = 0;
		while (FILE[charFileIndex] != 0)
			charFileIndex++;

		charFileIndex++; //So that charFileIndex is equal to the size of the FILE name

		//we build a char* containing all args and the file name
		char* tempArg = (char*) malloc((index + charFileIndex)*sizeof(char));
		memcpy(tempArg, FILE, charFileIndex*sizeof(char));
		memcpy(tempArg+charFileIndex, ARGUMENTS, index * sizeof(char));

		//We build the char** localArgv
		localArgv = (char**) malloc((count+1)*sizeof(char*));
		index = 0;
		for (int oneArg = 0; oneArg<count+1; oneArg++){
			localArgv[oneArg] = &(tempArg[index]);
			while (tempArg[index] != 0){
				index++;
			}
			index++;

		}

		//Value of localArgc is number of argument + the one from the file name
		localArgc = count + 1;
	}

	printf("There is %d arguments passed to simulator\n", localArgc);

	if (HELP || FILE == NULL){
		printf("Usage is %s [-v] file\n\t-v\tVerbose mode, prints all execution information\n", argv[0]);
		return 1;
	}

	//******************************************************************************************
	//Opening elf files
	ElfFile elfFile(FILE);
	RiscvSimulator* simulator = new RiscvSimulator();
	simulator->initialize(localArgc, localArgv);
	simulator->debugLevel = VERBOSE*2;

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
	simulator->pc = 0x10000;
	simulator->doSimulation(2000000000);

}
