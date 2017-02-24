#include <dbt/dbtPlateform.h>
#include <dbt/insertions.h>
#include <dbt/profiling.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <lib/endianness.h>
#include <lib/tools.h>
#include <simulator/vexSimulator.h>

#include <transformation/firstPassTranslator.h>
#include <transformation/irGenerator.h>
#include <transformation/optimizeBasicBlock.h>
#include <transformation/buildControlFlow.h>
#include <transformation/reconfigureVLIW.h>

#include <lib/debugFunctions.h>

#include <isa/vexISA.h>


#ifndef __NIOS
#include <lib/elfFile.h>
#endif

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


int translateOneSection(DBTPlateform &dbtPlateform, uint32 placeCode, int sectionStart, int startAddressSource, int endAddressSource){
	int previousPlaceCode = placeCode;
	uint32 size = (endAddressSource - startAddressSource)>>2;
	int addressStart = startAddressSource;
	placeCode = firstPassTranslator_RISCV(&dbtPlateform,
			size,
			sectionStart,
			addressStart,
			placeCode);


	//	debugFirstPassResult(dbtPlateform, previousPlaceCode+1, placeCode, addressStart);


		//We write back the result if needed
		void* destinationBinariesFile = openWriteFile((void*) "./binaries");
		unsigned int sizeBinaries = (placeCode<<4);

		return placeCode;
}


void readSourceBinaries(char* path, unsigned char *&code, unsigned int &addressStart, uint32 &size, DBTPlateform *platform){

#ifndef __NIOS
	//We open the elf file and search for the section that is of interest for us
	ElfFile elfFile(path);


	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);

		//The section we are looking for is called .text
		if (!section->getName().compare(".text")){

			code = section->getSectionCode();//0x3c
			addressStart = section->address + 0;
			size = section->size/4 - 0;

		}
	}


	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);

		if (section->address != 0){

			unsigned char* data = section->getSectionCode();
			platform->vexSimulator->initializeDataMemory(data, section->size, section->address);
			free(data);
		}
	}


#else
	read(0, &addressStart, sizeof(int));
	read(0, &size, sizeof(int));
	code = (unsigned char*) malloc(size*sizeof(unsigned char));

	for (int oneByte = 0; oneByte<size; oneByte++){
		read(0, &code[oneByte], sizeof(char));
	}

	size = size/4;
#endif
}

int run(DBTPlateform *platform, int nbCycle){
	return platform->vexSimulator->doStep(nbCycle);
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

	//Definition of objects used for DBT process
	DBTPlateform dbtPlateform;
	dbtPlateform.vexSimulator = new VexSimulator(dbtPlateform.vliwBinaries);
	unsigned char* code;
	unsigned int addressStart;
	uint32 size;

	//We read the binaries
	readSourceBinaries(argv[1], code, addressStart, size, &dbtPlateform);

	int numberOfSections = 1 + (size>>10);
	IRApplication application = IRApplication(numberOfSections);
	Profiler profiler = Profiler(&dbtPlateform);





	//we copy the binaries in the corresponding memory
	for (int oneInstruction = 0; oneInstruction<size; oneInstruction++)
		dbtPlateform.mipsBinaries[oneInstruction] = ((unsigned int*) code)[oneInstruction];

	//We declare the variable in charge of keeping a track of where we are writing
	uint32 placeCode = 0; //As 4 instruction bundle

	//We add initialization code to the vliw binaries
	placeCode = getInitCode(&dbtPlateform, placeCode, addressStart);
	placeCode = insertCodeForInsertions(&dbtPlateform, placeCode, addressStart);

	//We modify the initialization call
	writeInt(dbtPlateform.vliwBinaries, 0*16, assembleIInstruction(VEX_CALL, placeCode<<2, 63));

	initializeInsertionsMemory(size*4);

	/********************************************************
	 * First part of DBT: generating the first pass translation of binaries
	 *******************************************************
	 * TODO: description
	 *
	 *
	 ********************************************************/


	for (int oneSection=0; oneSection<(size>>10)+1; oneSection++){

		int startAddressSource = addressStart + oneSection*1024*4;
		int endAddressSource = startAddressSource + 1024*4;
		if (endAddressSource > addressStart + size*4)
			endAddressSource = addressStart + (size<<2);


		int effectiveSize = (endAddressSource - startAddressSource)>>2;
		for (int j = 0; j<effectiveSize; j++){
			dbtPlateform.mipsBinaries[j] = ((unsigned int*) code)[j+oneSection*1024];
		}
		int oldPlaceCode = placeCode;

		placeCode =  translateOneSection(dbtPlateform, placeCode, addressStart, startAddressSource,endAddressSource);

		buildBasicControlFlow(dbtPlateform, oneSection, startAddressSource, oldPlaceCode, placeCode, &application);


		//We select blocks for profiling:
		//If a block has more than 16 instructions, it is eligible for profiling.
		//TODO use a DEFINE instead of a fixed number of threshold
		for (int oneBlock = 0; oneBlock<application.numbersBlockInSections[oneSection]; oneBlock++){
			IRBlock *block = application.blocksInSections[oneSection][oneBlock];
			if (block->vliwEndAddress - block->vliwStartAddress>16){
				profiler.profileBlock(application.blocksInSections[oneSection][oneBlock]);
			}

		}
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
			initialDestination = initialDestination << 2; //This is compute the destination according to the #of instruction and not the number of 4-instr bundle
			writeInt(dbtPlateform.vliwBinaries, 16*(source), oldJump + ((initialDestination & 0x7ffff)<<7));

		}
		else{
			//In here we solve a relative jump

			indexOfDestination = destinationInVLIWFromNewMethod;
			initialDestination = destinationInVLIWFromNewMethod;

			initialDestination = initialDestination  - (source) ;
			initialDestination = initialDestination << 2; //This is compute the destination according to the #of instruction and not the number of 4-instr bundle

			//We modify the jump instruction to make it jump at the correct place
			writeInt(dbtPlateform.vliwBinaries, 16*(source), oldJump + ((initialDestination & 0x7ffff)<<7));

		}

		unsigned int instructionBeforePreviousDestination = readInt(dbtPlateform.vliwBinaries, 16*(indexOfDestination-1)+12);
		if (instructionBeforePreviousDestination != 0)
					writeInt(dbtPlateform.vliwBinaries, 16*(source+1)+12, instructionBeforePreviousDestination);
	}


	//We write back the result if needed
	void* destinationBinariesFile = openWriteFile((void*) "./binaries");
	unsigned int sizeBinaries = (placeCode<<4);


	//We initialize the VLIW processor with binaries and data from elf file
	#ifndef __NIOS
	dbtPlateform.vexSimulator->debugLevel = 2;
//	dbtPlateform.vexSimulator->debugLevel = 0;
	#endif



	//We also add information on insertions
	int insertionSize = 65536;
	int areaCodeStart=1;
	int areaStartAddress = 0;

	#ifndef __NIOS
	dbtPlateform.vexSimulator->initializeDataMemory((unsigned char*) insertionsArray, 65536*4, 0x7000000);
	#endif

	#ifndef __NIOS
	dbtPlateform.vexSimulator->initializeRun(0);
	#endif

	int runStatus=0;


	while (runStatus == 0){
		runStatus = run(&dbtPlateform, 1000);

		//If a profiled block is executed more than 10 times we optimize it and mark it as optimized
		for (int oneBlock = 0; oneBlock<profiler.getNumberProfiledBlocks(); oneBlock++){
			int profileResult = profiler.getProfilingInformation(oneBlock);
			IRBlock* block = profiler.getBlock(oneBlock);

			if (profileResult > 10 && block->blockState < IRBLOCK_STATE_SCHEDULED){
				fprintf(stderr, "Block from %d to %d is eligible to opti (%d exec)\n", block->vliwStartAddress, block->vliwEndAddress, profileResult);
				optimizeBasicBlock(block, &dbtPlateform, &application);
			}


//				if (profileResult > 20 && block->blockState == IRBLOCK_STATE_SCHEDULED){
//
//					fprintf(stderr, "Block from %d to %d is eligible advanced control flow building\n", block->vliwStartAddress, block->vliwEndAddress);
//					buildAdvancedControlFlow(&dbtPlateform, block, &application);
//					block->blockState = IRBLOCK_STATE_RECONF;
//					reconfigureVLIW(&dbtPlateform, application.procedures[application.numberProcedures-1]);
//					dbtPlateform.vexSimulator->initializeCodeMemory(dbtPlateform.vliwBinaries, sizeBinaries, 0);
//
//				}
		}

	}

	for (int oneBlock = 0; oneBlock<profiler.getNumberProfiledBlocks(); oneBlock++){
		IRBlock* block = profiler.getBlock(oneBlock);
		fprintf(stderr, "Block from %d to %d was executed %d times\n", block->vliwStartAddress, block->vliwEndAddress, profiler.getProfilingInformation(oneBlock));
	}

	//We print profiling result
	delete dbtPlateform.vexSimulator;
	free(code);


}

