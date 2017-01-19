#include <dbt/dbtPlateform.h>
#include <dbt/insertions.h>

#include <stdio.h>
#include <stdlib.h>

#include <lib/endianness.h>
#include <lib/tools.h>
#include <simulator/vexSimulator.h>

#include <transformation/firstPassTranslator.h>
#include <transformation/irGenerator.h>
#include <transformation/optimizeBasicBlock.h>
#include <transformation/buildControlFlow.h>

#include <lib/debugFunctions.h>

#include <isa/vexISA.h>


#ifndef __NO_LINUX_API

#include <lib/elfFile.h>

void printStats(unsigned int size, short* blockBoundaries){

	float numberBlocks = 0;

	for (int oneInstruction = 0; oneInstruction < size; oneInstruction++){
		if (blockBoundaries[oneInstruction] == 1)
			numberBlocks++;
	}

	printf("\n* Statistics on used binaries:\n");
	printf("* \tThere is %d instructions.\n", size);
	printf("* \tThere is %d blocks.\n", (int) numberBlocks);
	printf("* \tBlocks mean size is %f.\n\n", size/numberBlocks);

}


int translateOneSection(DBTPlateform &dbtPlateform, ac_int<32, false> placeCode, int sectionStart, int startAddressSource, int endAddressSource){
	int previousPlaceCode = placeCode;
	ac_int<32, false> size = (endAddressSource - startAddressSource)>>2;
	int addressStart = startAddressSource;
	firstPassTranslator_RISCV(dbtPlateform.mipsBinaries,
			&size,
			sectionStart,
			addressStart,
			dbtPlateform.vliwBinaries,
			&placeCode,
			dbtPlateform.insertions,
			dbtPlateform.blockBoundaries,
			dbtPlateform.procedureBoundaries);


	//	debugFirstPassResult(dbtPlateform, previousPlaceCode+1, placeCode, addressStart);

		IRProcedure *controlFlow;
		int nbProc = 0;//buildBasicControlFlow(dbtPlateform, previousPlaceCode, placeCode, &controlFlow);


		//We write back the result if needed
		void* destinationBinariesFile = openWriteFile((void*) "./binaries");
		unsigned int sizeBinaries = (placeCode<<4);

		return placeCode;
}


int main(int argc, char *argv[])
{

	/***********************************
	 *  Initialization of the DBT platform
	 ***********************************
	 * In the linux implementation, this is done by reading an elf file and copying binary instructions
	 * in the corresponding memory.
	 * In a real platform, this may require no memory initialization as the binaries would already be stored in the
	 * system memory.
	 ************************************/

	//We open the elf file and search for the section that is of interest for us
	ElfFile elfFile((char *) argv[1]);

	unsigned char* code;
	unsigned int addressStart;
	ac_int<32, false> size;

	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);

		//The section we are looking for is called .text
		if (!section->getName().compare(".text")){

			code = &(section->getSectionCode()[0]); //0x3c
			addressStart = section->address + 0;
			size = section->size/4 - 0;

		}
	}

	DBTPlateform dbtPlateform;

	//we copy the binaries in the corresponding memory
	for (int oneInstruction = 0; oneInstruction<size; oneInstruction++)
		dbtPlateform.mipsBinaries[oneInstruction] = ((unsigned int*) code)[oneInstruction];

	//We declare the variable in charge of keeping a track of where we are writing
	ac_int<32, false> placeCode = 0; //As 4 instruction bundle

	//We add initialization code to the vliw binaries
	placeCode = getInitCode(dbtPlateform.vliwBinaries, placeCode, addressStart);
	placeCode = insertCodeForInsertions(dbtPlateform.vliwBinaries, placeCode, addressStart);

	//We modify the initialization call
	writeInt(dbtPlateform.vliwBinaries, 0*16, assembleIInstruction(VEX_CALL, placeCode, 63));

	initializeInsertionsMemory(size*4);

	/********************************************************
	 * First part of DBT: generating the first pass translation of binaries
	 *******************************************************
	 * TODO: description
	 *
	 *
	 ********************************************************/

//We launch the actual generation
	for (int i=0; i<(size>>10)+1; i++){

		int startAddressSource = addressStart + i*1024*4;
		int endAddressSource = startAddressSource + 1024*4;
		if (endAddressSource > addressStart + size*4)
			endAddressSource = addressStart + (size<<2);


		int effectiveSize = (endAddressSource - startAddressSource)>>2;
		for (int j = 0; j<effectiveSize; j++){
			dbtPlateform.mipsBinaries[j] = ((unsigned int*) code)[j+i*1024];
		}

		placeCode =  translateOneSection(dbtPlateform, placeCode, addressStart, startAddressSource,endAddressSource);


	}
	for (int oneUnresolvedJump = 0; oneUnresolvedJump<unresolvedJumpsArray[0]; oneUnresolvedJump++){
		unsigned int source = unresolvedJumpsSourceArray[oneUnresolvedJump+1];
		unsigned int initialDestination = unresolvedJumpsArray[oneUnresolvedJump+1];
		unsigned char type = unresolvedJumpsTypeArray[oneUnresolvedJump+1];

		unsigned int oldJump = readInt(dbtPlateform.vliwBinaries, 16*(source));
		unsigned int indexOfDestination = 0;

		unsigned int destinationInVLIWFromNewMethod = solveUnresolvedJump(initialDestination);
		if (destinationInVLIWFromNewMethod == -1){
			printf("A jump from %d to %x is still unresolved... (%d insertions)\n", source, initialDestination, insertionsArray[(initialDestination>>10)<<11]);
		}
		else if (type == UNRESOLVED_JUMP_ABSOLUTE){
			//In here we solve an absolute jump
			indexOfDestination = destinationInVLIWFromNewMethod;
			initialDestination = destinationInVLIWFromNewMethod;
			writeInt(dbtPlateform.vliwBinaries, 16*(source), oldJump + ((initialDestination & 0x7ffff)<<7));

		}
		else{
			//In here we solve a relative jump

			indexOfDestination = destinationInVLIWFromNewMethod;
			initialDestination = destinationInVLIWFromNewMethod;

			initialDestination = initialDestination  - (source) ;

			//We modify the jump instruction to make it jump at the correct place
			writeInt(dbtPlateform.vliwBinaries, 16*(source), oldJump + ((initialDestination & 0x7ffff)<<7));

		}

		unsigned int instructionBeforePreviousDestination = readInt(dbtPlateform.vliwBinaries, 16*(indexOfDestination-1)+12);
		if (instructionBeforePreviousDestination != 0)
					writeInt(dbtPlateform.vliwBinaries, 16*(source+1)+12, instructionBeforePreviousDestination);
	}


//	int previousPlaceCode = placeCode;
//	firstPassTranslator_RISCV(dbtPlateform.mipsBinaries,
//			&size,
//			addressStart,
//			dbtPlateform.vliwBinaries,
//			&placeCode,
//			dbtPlateform.insertions,
//			dbtPlateform.blockBoundaries,
//			dbtPlateform.procedureBoundaries);


//	debugFirstPassResult(dbtPlateform, previousPlaceCode+1, placeCode, addressStart);

	IRProcedure *controlFlow;
	int nbProc = 0;//buildBasicControlFlow(dbtPlateform, previousPlaceCode, placeCode, &controlFlow);


	//We write back the result if needed
	void* destinationBinariesFile = openWriteFile((void*) "./binaries");
	unsigned int sizeBinaries = (placeCode<<4);


//	for (int i=0;i<sizeBinaries>>4;i++){
//		printf("0x%xl, 0x%xl, 0x%xl, 0x%xl,\n", (int) dbtPlateform.vliwBinaries[i].slc<32>(0),
//				(int) dbtPlateform.vliwBinaries[i].slc<32>(32),
//				(int) dbtPlateform.vliwBinaries[i].slc<32>(64),
//				(int) dbtPlateform.vliwBinaries[i].slc<32>(96));
//	}

//	optimizeBasicBlock(273, 355, &dbtPlateform);
//	optimizeBasicBlock(163, 286, &dbtPlateform);


	//We initialize the VLIW processor with binaries and data from elf file
	VexSimulator *simulator = new VexSimulator();
	simulator->debugLevel = 2;

	simulator->initializeCodeMemory(dbtPlateform.vliwBinaries, sizeBinaries, 0);
	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);

		if (section->address != 0){

			unsigned char* data = section->getSectionCode();
			simulator->initializeDataMemory(data, section->size, section->address);
			free(data);
		}
	}



	//We also add information on insertions
	int insertionSize = 65536;
	int areaCodeStart=1;
	int areaStartAddress = 0;

	simulator->initializeDataMemory((unsigned char*) insertionsArray, 65536*4, 0x7000000); //TODO

	//if (simulator->debugLevel == 1)
//		for (int oneInsertion = 1; oneInsertion<=dbtPlateform.insertions[0]; oneInsertion++)
//			fprintf(stderr, "insert;%d\n",(int) dbtPlateform.insertions[oneInsertion]);
		fprintf(stderr, "insert;%d\n", (int) placeCode);

	simulator->run(0);
	delete simulator;


	/********************************************************/
	/********************************************************/


	/********************************************************/
	/* Second part, generation of the bytecode w/o regalloc */
	/********************************************************/
/*	printf("\n**** Generation & Compilation & Execution of Bytecode v1 \n");

	unsigned char bytecode[16000];
	unsigned int placeBytecode = 0;


	irGenerator(dbtPlateform.vliwBinaries,
			&size,
			addressStart,
			dbtPlateform.bytecode,
			&placeBytecode,
			dbtPlateform.blockBoundaries,
			dbtPlateform.procedureBoundaries,
			dbtPlateform.insertions);

	//We write back the result if needed
	void* destinationBytecodeFile = openWriteFile((void*) "./bytecode.bc");
	unsigned int sizeBytecode = placeBytecode;

//	for (int i=0;i<sizeBinaries>>4;i++){
//		printf("%d : 0x%xl, 0x%xl, 0x%xl, 0x%xl,\n", i,((unsigned int*) destinationBinaries)[4*i+0],
//				((unsigned int*) destinationBinaries)[4*i+1],
//				((unsigned int*) destinationBinaries)[4*i+2],
//				((unsigned int*) destinationBinaries)[4*i+3]);
//	}

	writeFile(destinationBytecodeFile, 0, sizeBytecode, 1, bytecode);
	closeFile(destinationBytecodeFile);

//	JITCompilation((void*) "./bytecode.bc",(void*) "./binaries");
//
//	void* binariesFile = openReadFile((void*)"./binaries");
//	unsigned int* binariesSize = (unsigned int*) readFile(binariesFile, 0, 4, 1);
//	unsigned char* postSchedulingBinaries = (unsigned char*) readFile(binariesFile, 4, 1, *binariesSize<<2);
//
////	for (int i=0;i<*binariesSize>>2;i++){
////		printf("0x%xl, 0x%xl, 0x%xl, 0x%xl,\n", ((unsigned int*) postSchedulingBinaries)[4*i+0],
////				((unsigned int*) postSchedulingBinaries)[4*i+1],
////				((unsigned int*) postSchedulingBinaries)[4*i+2],
////				((unsigned int*) postSchedulingBinaries)[4*i+3]);
////	}
	//We initialize the VLIW processor with binaries and data from elf file
	VexSimulator *simulator2 = new VexSimulator();
	simulator2->debugLevel = 0;

	simulator2->initializeCodeMemory(destinationBinaries, sizeBinaries, 0);


	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);
		if (!section->getName().compare(".data")){
			unsigned char* data = section->getSectionCode();
			simulator2->initializeDataMemory(data, section->size, section->address);
			free(data);
		}
	}

	simulator2->run(0);
	delete simulator2;*/

//	/********************************************************/
//	/********************************************************/
//
//	printf("\n**** Generation & Compilation & Execution of Bytecode v2 \n");
//
//	unsigned char bytecodeRenamed[16000];
//	unsigned int placeBytecodeRenamed = 0;
//
//
//
//
////	for (unsigned int oneInstruction = 0; oneInstruction<size; oneInstruction++){
////		printf("At %x : %d    %d\n", oneInstruction*4 +addressStart , blocksBoundaries[oneInstruction], proceduresBoundaries[oneInstruction]);
////	}
//	generateRenamedBytecode(code, &size, addressStart,
//			bytecodeRenamed, &placeBytecodeRenamed,
//			blocksBoundaries, proceduresBoundaries);
//
//
//
//	//We write back the result if needed
//	void* destinationBytecodeRenamedFile = openWriteFile((void*) "./bytecodeRenamed.bc");
//	unsigned int sizeBytecodeRenamed = placeBytecodeRenamed;
//
//
//
//	writeFile(destinationBytecodeRenamedFile, 0, sizeBytecodeRenamed, 1, bytecodeRenamed);
//	closeFile(destinationBytecodeRenamedFile);
//
//	JITCompilation((void*) "./bytecodeRenamed.bc",(void*) "./binaries");
//
//	void* binariesRenamedFile = openReadFile((void*)"./binaries");
//	unsigned int* binariesRenamedSize = (unsigned int*) readFile(binariesRenamedFile, 0, 4, 1);
//	unsigned char* postSchedulingRenamedBinaries = (unsigned char*) readFile(binariesRenamedFile, 4, 1, *binariesRenamedSize<<2);
//
////	for (int i=0;i<*binariesSize>>2;i++){
////		printf("0x%xl, 0x%xl, 0x%xl, 0x%xl,\n", ((unsigned int*) postSchedulingRenamedBinaries)[4*i+0],
////				((unsigned int*) postSchedulingRenamedBinaries)[4*i+1],
////				((unsigned int*) postSchedulingRenamedBinaries)[4*i+2],
////				((unsigned int*) postSchedulingRenamedBinaries)[4*i+3]);
////	}
//	//We initialize the VLIW processor with binaries and data from elf file
//	VexSimulator *simulator3 = new VexSimulator();
//	simulator3->initializeCodeMemory(postSchedulingRenamedBinaries, *binariesRenamedSize<<2, 0);
//
//	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
//		ElfSection *section = elfFile.sectionTable->at(sectionCounter);
//		if (!section->getName().compare(".data"))
//			simulator3->initializeDataMemory(section->getSectionCode(), section->size, section->address);
//
//	}
//
////	simulator3->run(0);
//	delete simulator3;
//
//
//  return 0;
}

#endif

#ifdef __NO_LINUX_API
#include <binaries_content.h>

int main(int argc, char *argv[])
{

	unsigned char destination[65536];

	unsigned char destinationBinaries[65536];
	unsigned int placeCode = 2; //As 4 instruction bundle
	/********************************************************/
	/* First part, generation of inefficient code           */
	/********************************************************/
	startPerformances(0);


	//We define data structures to find block/procedure boundaries. This will be used on next stage
	short *blocksBoundaries = (short*) malloc(size * sizeof(short));
	short *proceduresBoundaries = (short*) malloc(size * sizeof(short));

	for (int i = 0; i<size; i++){
		blocksBoundaries[i] = 0;
		proceduresBoundaries[i] =0;
	}

	unsigned int numberInsertions = 0;
	int insertions[512];

	//We launch the actual generation
	generateInterpretationBinaries(code, &size, addressStart, destinationBinaries, &placeCode, numberInsertions, insertions, blocksBoundaries, proceduresBoundaries);



	//We write back the result if needed
	unsigned int sizeBinaries = (placeCode>>2);


	/********************************************************/
	/********************************************************/


	/********************************************************/
	/* Second part, generation of the bytecode w/o regalloc */
	/********************************************************/

	unsigned char bytecode[16000];
	for (int i = 0; i<16000; i++)
		bytecode[i] = 0;
	unsigned int placeBytecode = 0;

	DBTFrontend(code, &size, addressStart,
			bytecode, &placeBytecode,
			blocksBoundaries, proceduresBoundaries);

	//We write back the result if needed
	void* destinationBytecodeFile = openWriteFile((void*) "./bytecode.bc");
	unsigned int sizeBytecode = placeBytecode;



	writeFile(destination, 0, sizeBinaries<<4, 1, bytecode);

	stopPerformances(0);

	printf("Time taken for generation %d after step1 and %d after step 2 cycles\n", sizeBinaries, getPerformances(0));


  return 0;
}

#endif
