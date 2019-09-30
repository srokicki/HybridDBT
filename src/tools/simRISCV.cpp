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
	char* binaryFile = NULL;
	char* ARGUMENTS = NULL;
	FILE** inStreams = (FILE**) malloc(10*sizeof(FILE*));
	FILE** outStreams = (FILE**) malloc(10*sizeof(FILE*));

	int nbInStreams = 0;
	int nbOutStreams = 0;

	while ((c = getopt (argc, argv, "vhf:a:o:i:")) != -1)
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
	  case 'f':
		  binaryFile = optarg;
	  break;
	  case 'i':
		  if (strcmp(optarg, "stdin") == 0)
			  inStreams[nbInStreams] = stdin;
		  else
			  inStreams[nbInStreams] = fopen(optarg, "r");
		  nbInStreams++;
	  break;
	  case 'o':
		  if (strcmp(optarg, "stdout") == 0)
			  outStreams[nbOutStreams] = stdout;
		  else if (strcmp(optarg, "stderr") == 0)
			  outStreams[nbOutStreams] = stderr;
		  else
			  outStreams[nbOutStreams] = fopen(optarg, "w");
		  nbOutStreams++;
	  break;
	  default:
		abort ();
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
			}
			index++;
		}
		index++; //so that index is equal to the size of the char*


		//We find size of filename
		int charFileIndex = 0;
		while (binaryFile[charFileIndex] != 0)
			charFileIndex++;

		charFileIndex++; //So that charFileIndex is equal to the size of the FILE name

		//we build a char* containing all args and the file name
		char* tempArg = (char*) malloc((index + charFileIndex)*sizeof(char));
		memcpy(tempArg, binaryFile, charFileIndex*sizeof(char));
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


	if (HELP || binaryFile == NULL){
		printf("Usage is %s [-v] file\n\t-v\tVerbose mode, prints all execution information\n", argv[0]);
		return 1;
	}

	//******************************************************************************************
	//Opening elf files
	fprintf(stderr, "Binary file is %s\n", binaryFile);
	ElfFile elfFile(binaryFile);
	RiscvSimulator* simulator = new RiscvSimulator();
	simulator->initialize(localArgc, localArgv);
	simulator->debugLevel = VERBOSE*2;
	simulator->inStreams = inStreams;
	simulator->nbInStreams = nbInStreams;
	simulator->outStreams = outStreams;
	simulator->nbOutStreams = nbOutStreams;

	unsigned int heapAddress = 0;
	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *oneSection = elfFile.sectionTable->at(sectionCounter);

		if (oneSection->address != 0){
			//If the address is not null we place its content into memory
			unsigned char* sectionContent = oneSection->getSectionCode();

			for (unsigned int byteNumber = 0; byteNumber<oneSection->size; byteNumber++){
				simulator->stb(oneSection->address + byteNumber, sectionContent[byteNumber]);
			}

			if (oneSection->address + oneSection->size > heapAddress)
				heapAddress = oneSection->address + oneSection->size;
		}
	}
	simulator->heapAddress = heapAddress;
	simulator->pc = 0x10000;

	for (int oneSymbol = 0; oneSymbol < elfFile.symbols->size(); oneSymbol++){
		ElfSymbol *symbol = elfFile.symbols->at(oneSymbol);
		const char* name = (const char*) &(elfFile.sectionTable->at(elfFile.indexOfSymbolNameSection)->getSectionCode()[symbol->name]);

		if (strcmp(name, "_start") == 0){
			fprintf(stderr, "%s\n", name);
			simulator->pc = symbol->offset;

		}
	}
	fprintf(stderr, "PC start is %x\n", (unsigned int) simulator->pc);

	int error = simulator->doSimulation(50000000);

	if (error){
		fprintf(stdout,"Simulation finished ! \n\t Number of cycles: 0 \n\t Number of instruction executed: 0\n");

	}
	else{
		fprintf(stdout,"Simulation finished ! \n\t Number of cycles: %d \n\t Number of instruction executed: %d\n", (int) simulator->cycle, (int) simulator->n_inst);
	}
}
