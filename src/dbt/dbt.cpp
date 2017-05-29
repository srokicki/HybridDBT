#include <dbt/dbtPlateform.h>
#include <dbt/insertions.h>
#include <dbt/profiling.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <lib/endianness.h>
#include <lib/tools.h>
#include <simulator/vexSimulator.h>
#include <simulator/riscvSimulator.h>

#include <transformation/firstPassTranslator.h>
#include <transformation/irGenerator.h>
#include <transformation/optimizeBasicBlock.h>
#include <transformation/buildControlFlow.h>
#include <transformation/reconfigureVLIW.h>
#include <transformation/buildTraces.h>

#include <lib/debugFunctions.h>

#include <isa/vexISA.h>
#include <isa/riscvISA.h>


#ifndef __NIOS
#include <lib/elfFile.h>
#else
#include <system.h>
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
//	read(0, &addressStart, sizeof(int));
//	read(0, &size, sizeof(int));
	addressStart = tmp_addressStart;
	size = tmp_size;
	code = tmp_code;
//	code = (unsigned char*) malloc(size*sizeof(unsigned char));
//
//	for (int oneByte = 0; oneByte<size; oneByte++){
//		read(0, &code[oneByte], sizeof(char));
//	}

	size = size/4;
#endif
}
int cnt = 0;
char align[70000];
int run(DBTPlateform *platform, int nbCycle){
#ifndef __NIOS

//	int res;
//	for (int i=0; i<nbCycle; i++){
//	fprintf(stderr, "Run %d  (%lx - %lx) ", cnt++,  (uint64_t) platform->vexSimulator->PC/4,(uint64_t) platform->riscvSimulator->pc);
//	char isIns = align[platform->vexSimulator->PC >> 2];
//	uint32 instr = platform->riscvSimulator->ldw(platform->riscvSimulator->pc);
//	if (!isIns)
//		platform->riscvSimulator->doSimulation(1);
//	res = platform->vexSimulator->doStep(1);
//
//	std::cerr << printDecodedInstr(platform->vexSimulator->ftoDC1.instruction);
//	std::cerr << ";";
//	std::cerr << printDecodedInstr(platform->vexSimulator->ftoDC2.instruction);
//	std::cerr << ";";
//	std::cerr << printDecodedInstr(platform->vexSimulator->ftoDC3.instruction);
//	std::cerr << ";";
//	std::cerr << printDecodedInstr(platform->vexSimulator->ftoDC4.instruction);
//	std::cerr << ";";
//	std::cerr << printDecodedInstrRISCV(instr);
//	std::cerr << ";";
//
//	if (!isIns)
//		for (int oneReg=3; oneReg<31; oneReg++)
//			if (platform->vexSimulator->REG[oneReg] != platform->riscvSimulator->REG[oneReg])
//				fprintf(stderr, "r%d:%lx!=%lx",oneReg, (uint64_t) platform->vexSimulator->REG[oneReg], (uint64_t)  platform->riscvSimulator->REG[oneReg]);
//
//
//	fprintf(stderr, "\n");
//
//	if (res)
//		break;
//	}
//
//	return res;

	return platform->vexSimulator->doStep(nbCycle);

#else
	printf("starting run\n");

//#define ALT_CI_COMPONENT_RUN_0(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_RUN_0_N,(A),(B))
//#define ALT_CI_COMPONENT_RUN_0_1(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_RUN_0_1_N,(A),(B))
//#define ALT_CI_COMPONENT_RUN_0_1_N 0x4
//#define ALT_CI_COMPONENT_RUN_0_2(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_RUN_0_2_N,(A),(B))
//#define ALT_CI_COMPONENT_RUN_0_2_N 0x5
//#define ALT_CI_COMPONENT_RUN_0_3(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_RUN_0_3_N,(A),(B))

	int start = 0;
	 ALT_CI_COMPONENT_RUN_0(0,0);
	 int status = ALT_CI_COMPONENT_RUN_0_2(0,0);
	 if ((status & 0x3) == 1)
		 return 0;
	 ALT_CI_COMPONENT_RUN_0_1(0,0);
	 ALT_CI_COMPONENT_RUN_0(0,0);
	 return (ALT_CI_COMPONENT_RUN_0_3(0,0))>>2;


#endif

}





int main(int argc, char *argv[])
{

	/*Parsing arguments of the commant
	 *
	 */

	int c;
	int VERBOSE = 0;
	int OPTLEVEL = 1;
	int HELP = 0;
	char* FILE = NULL;
	char* ARGUMENTS = NULL;

	while ((c = getopt (argc, argv, "vOha:")) != -1)
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
	  case 'O':
		  OPTLEVEL = atoi(argv[optind]);
		break;
	  default:
		abort ();
	  }

	for (int index = optind; index < argc; index++)
		FILE = argv[index];

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


	if (HELP || FILE == NULL){
		printf("Usage is %s [-v] [-On] file\n\t-v\tVerbose mode, prints all execution information\n\t-On\t Optimization level from zero to two\n", argv[0]);
		return 1;
	}

	// We translate OPTLEVEL

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

	#ifndef __NIOS
	dbtPlateform.vexSimulator = new VexSimulator(dbtPlateform.vliwBinaries);
	#endif

	unsigned char* code;
	unsigned int addressStart;
	uint32 size;

	//We read the binaries
	readSourceBinaries(FILE, code, addressStart, size, &dbtPlateform);

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

	for (int oneInsertion=0; oneInsertion<placeCode; oneInsertion++){
		align[oneInsertion] = 1;
		if (VERBOSE)
			fprintf(stderr, "insert;%d\n", oneInsertion);
	}


	//Test debug
//	ElfFile elfFile(FILE);
//	dbtPlateform.riscvSimulator = new RiscvSimulator();
//	dbtPlateform.riscvSimulator->initialize(localArgc, localArgv);
//	dbtPlateform.riscvSimulator->debugLevel = VERBOSE*2;
//
//	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
//		ElfSection *oneSection = elfFile.sectionTable->at(sectionCounter);
//
//		if (oneSection->address != 0){
//			//If the address is not null we place its content into memory
//			unsigned char* sectionContent = oneSection->getSectionCode();
//
//			for (unsigned int byteNumber = 0; byteNumber<oneSection->size; byteNumber++){
//				dbtPlateform.riscvSimulator->stb(oneSection->address + byteNumber, sectionContent[byteNumber]);
//			}
//		}
//	}
//
//	dbtPlateform.riscvSimulator->pc = 0x10000;

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

			if (block->vliwEndAddress - block->vliwStartAddress>7){
				profiler.profileBlock(application.blocksInSections[oneSection][oneBlock]);
			}

		}

		int** insertions = (int**) malloc(sizeof(int **));
		int nbIns = getInsertionList(oneSection*1024, insertions);
		for (int oneInsertion=0; oneInsertion<nbIns; oneInsertion++){
			if (VERBOSE)
				fprintf(stderr, "insert;%d\n", (*insertions)[oneInsertion]+(*insertions)[-1]);
			align[((*insertions)[oneInsertion]+(*insertions)[-1])] = 1;

		}
	}

	for (int oneUnresolvedJump = 0; oneUnresolvedJump<unresolvedJumpsArray[0]; oneUnresolvedJump++){
		unsigned int source = unresolvedJumpsSourceArray[oneUnresolvedJump+1];
		unsigned int initialDestination = unresolvedJumpsArray[oneUnresolvedJump+1];
		unsigned int type = unresolvedJumpsTypeArray[oneUnresolvedJump+1];

		unsigned int oldJump = readInt(dbtPlateform.vliwBinaries, 16*(source));
		unsigned int indexOfDestination = 0;
		unsigned char isAbsolute = ((type & 0x7f) != VEX_BR) && ((type & 0x7f) != VEX_BRF);

		unsigned int destinationInVLIWFromNewMethod = solveUnresolvedJump(initialDestination);
		if (destinationInVLIWFromNewMethod == -1){
			printf("A jump from %d to %x is still unresolved... (%d insertions)\n", source, initialDestination, insertionsArray[(initialDestination>>10)<<11]);
		}
		else if (isAbsolute){
			//In here we solve an absolute jump
			indexOfDestination = destinationInVLIWFromNewMethod;
			initialDestination = destinationInVLIWFromNewMethod;
			initialDestination = initialDestination << 2; //This is compute the destination according to the #of instruction and not the number of 4-instr bundle
			writeInt(dbtPlateform.vliwBinaries, 16*(source), type + ((initialDestination & 0x7ffff)<<7));

		}
		else{
			//In here we solve a relative jump
			indexOfDestination = destinationInVLIWFromNewMethod;
			initialDestination = destinationInVLIWFromNewMethod;

			initialDestination = initialDestination  - (source) ;
			initialDestination = initialDestination << 2; //This is compute the destination according to the #of instruction and not the number of 4-instr bundle

			//We modify the jump instruction to make it jump at the correct place
			writeInt(dbtPlateform.vliwBinaries, 16*(source), type + ((initialDestination & 0x7ffff)<<7));

		}

		unsigned int instructionBeforePreviousDestination = readInt(dbtPlateform.vliwBinaries, 16*(indexOfDestination-1)+12);
		if (instructionBeforePreviousDestination != 0)
					writeInt(dbtPlateform.vliwBinaries, 16*(source+1)+12, instructionBeforePreviousDestination);
	}



	//We initialize the VLIW processor with binaries and data from elf file
	#ifndef __NIOS
	if (VERBOSE)
		dbtPlateform.vexSimulator->debugLevel = 2;
	#endif



	//We also add information on insertions
	int insertionSize = 65536;
	int areaCodeStart=1;
	int areaStartAddress = 0;

	#ifndef __NIOS
	dbtPlateform.vexSimulator->initializeDataMemory((unsigned char*) insertionsArray, 65536*4, 0x7000000);
	#endif

	#ifndef __NIOS
	dbtPlateform.vexSimulator->initializeRun(0, localArgc, localArgv);
	#endif

//	for (int i=0;i<placeCode;i++){
//		fprintf(stderr, "%d ", i*4);
//		std::cerr << printDecodedInstr(dbtPlateform.vliwBinaries[i].slc<32>(0)); fprintf(stderr, " ");
//		std::cerr << printDecodedInstr(dbtPlateform.vliwBinaries[i].slc<32>(32)); fprintf(stderr, " ");
//		std::cerr << printDecodedInstr(dbtPlateform.vliwBinaries[i].slc<32>(64)); fprintf(stderr, " ");
//		std::cerr << printDecodedInstr(dbtPlateform.vliwBinaries[i].slc<32>(96)); fprintf(stderr, "\n");
//
//	}

	int runStatus=0;
	int abortCounter = 0;
	int scheduleCounter = 0;

	while (runStatus == 0){
		runStatus = run(&dbtPlateform, 1000);
		abortCounter++;
		if (abortCounter>8000000)
			break;

		if (VERBOSE)
			fprintf(stderr, "IPC;%f\n", dbtPlateform.vexSimulator->getAverageIPC());

		//If a profiled block is executed more than 10 times we optimize it and mark it as optimized
		for (int oneBlock = 0; oneBlock<profiler.getNumberProfiledBlocks(); oneBlock++){
			int profileResult = profiler.getProfilingInformation(oneBlock);
			IRBlock* block = profiler.getBlock(oneBlock);

			if (OPTLEVEL >= 1 && profileResult > 10 && block->blockState < IRBLOCK_STATE_SCHEDULED){
				optimizeBasicBlock(block, &dbtPlateform, &application, placeCode);
				scheduleCounter++;
			}


				if (OPTLEVEL >= 2 && profileResult > 20 && block->blockState == IRBLOCK_STATE_SCHEDULED){

					fprintf(stderr, "Block from %d to %d is eligible advanced control flow building\n", block->vliwStartAddress, block->vliwEndAddress);
					buildAdvancedControlFlow(&dbtPlateform, block, &application);
					block->blockState = IRBLOCK_STATE_RECONF;
					buildTraces(&dbtPlateform, application.procedures[application.numberProcedures-1]);
					//placeCode = reconfigureVLIW(&dbtPlateform, application.procedures[application.numberProcedures-1], placeCode);

				}
		}

	}

	//We write back the result if needed
	void* destinationBinariesFile = openWriteFile((void*) "./binaries");
	unsigned int sizeBinaries = (placeCode<<4);
//	for (int i=0;i<placeCode;i++){
//		fprintf(stderr, "%d ", i*4);
//		std::cerr << printDecodedInstr(dbtPlateform.vliwBinaries[i].slc<32>(0)); fprintf(stderr, " ");
//		std::cerr << printDecodedInstr(dbtPlateform.vliwBinaries[i].slc<32>(32)); fprintf(stderr, " ");
//		std::cerr << printDecodedInstr(dbtPlateform.vliwBinaries[i].slc<32>(64)); fprintf(stderr, " ");
//		std::cerr << printDecodedInstr(dbtPlateform.vliwBinaries[i].slc<32>(96)); fprintf(stderr, "\n");
//
//	}

	for (int oneBlock = 0; oneBlock<profiler.getNumberProfiledBlocks(); oneBlock++){
		IRBlock* block = profiler.getBlock(oneBlock);
//		fprintf(stderr, "Block from %d to %d was executed %d times\n", block->vliwStartAddress, block->vliwEndAddress, profiler.getProfilingInformation(oneBlock));
	}

	//We print profiling result
	#ifndef __NIOS
	delete dbtPlateform.vexSimulator;
	free(code);
	#endif


}

