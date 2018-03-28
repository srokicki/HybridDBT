#include <dbt/dbtPlateform.h>
#include <dbt/insertions.h>
#include <dbt/profiling.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <lib/endianness.h>
#include <simulator/vexSimulator.h>
#include <simulator/vexCgraSimulator.h>
#include <simulator/vexTraceSimulator.h>
#include <simulator/riscvSimulator.h>

#include <transformation/irGenerator.h>
#include <transformation/optimizeBasicBlock.h>
#include <transformation/buildControlFlow.h>
#include <transformation/reconfigureVLIW.h>
#include <transformation/buildTraces.h>
#include <transformation/rescheduleProcedure.h>
#include <transformation/cgraScheduler.h>
#include <transformation/localRegisterAnalysis.h>
#include <lib/debugFunctions.h>

#include <isa/vexISA.h>
#include <isa/riscvISA.h>
#include <isa/cgraIsa.h>

#include <lib/config.h>
#include <lib/log.h>
#include <lib/traceQueue.h>
#include <lib/threadedDebug.h>
#include <transformation/firstPassTranslation.h>

#include <lib/pcmWrapper.h>

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

	Log::printf(0, "\n* Statistics on used binaries:\n");
	Log::printf(0, "* \tThere is %d instructions.\n", size);
	Log::printf(0, "* \tThere is %d blocks.\n", (int) numberBlocks);
	Log::printf(0, "* \tBlocks mean size is %f.\n\n", size/numberBlocks);

}


int translateOneSection(DBTPlateform &dbtPlateform, unsigned int placeCode, int sourceStartAddress, int sectionStartAddress, int sectionEndAddress){
	int previousPlaceCode = placeCode;
	unsigned int size = (sectionEndAddress - sectionStartAddress)>>2;
	placeCode = firstPassTranslator(&dbtPlateform,
			size,
			sourceStartAddress,
			sectionStartAddress,
			placeCode);



		return placeCode;
}


void readSourceBinaries(char* path, unsigned char *&code, unsigned int &addressStart, unsigned int &size, unsigned int &pcStart, DBTPlateform *platform){

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

			if (size > MEMORY_SIZE){
				Log::fprintf(0, stderr, "Error: binary file has %d instructions, we currently handle a maximum size of %d\n", size, MEMORY_SIZE);
				exit(-1);
			}


		}
	}

	unsigned int localHeapAddress = 0;
	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);

		if (section->address != 0){

			unsigned char* data = section->getSectionCode();
			platform->vexSimulator->initializeDataMemory(data, section->size, section->address);
			free(data);

			if (section->address + section->size > localHeapAddress)
				localHeapAddress = section->address + section->size;
		}
	}
	platform->vexSimulator->heapAddress = localHeapAddress;

	for (int oneSymbol = 0; oneSymbol < elfFile.symbols->size(); oneSymbol++){
		ElfSymbol *symbol = elfFile.symbols->at(oneSymbol);
		const char* name = (const char*) &(elfFile.sectionTable->at(elfFile.indexOfSymbolNameSection)->getSectionCode()[symbol->name]);

		if (strcmp(name, "_start") == 0){
			pcStart = symbol->offset;

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
int run(DBTPlateform *platform, int nbCycle){
#ifndef __NIOS


	return platform->vexSimulator->doStep(nbCycle);

#else
	Log::printf(0, "starting run\n");

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

	int cgraConfs = 0;

	/*Parsing arguments of the commant
	 *
	 */

	static std::map<std::string, FILE*> ios =
	{
	{ "stdin", stdin },
	{ "stdout", stdout },
	{ "stderr", stderr }
	};

	Config cfg(argc, argv);

	if (cfg.has("papi"))
		PcmWrapper::init();

	CgraScheduler * scheduler = cfg.has("edge") ? (CgraScheduler*)new EdgeCentricScheduler() : (CgraScheduler*)new NodeCentricScheduler();

	int CONFIGURATION = cfg.has("c") ? std::stoi(cfg["c"]) : 2;

	int VERBOSE = cfg.has("v") ? std::stoi(cfg["v"]) : 0;
	int STATMODE = cfg.has("statmode") ? std::stoi(cfg["statmode"]) : 0;


	Log::Init(VERBOSE, STATMODE);

	int OPTLEVEL = cfg.has("O") ? std::stoi(cfg["O"]) : 2;
	char* binaryFile = cfg.has("f") && !cfg["f"].empty() ? (char*)cfg["f"].c_str() : NULL;

	FILE** inStreams = (FILE**) malloc(10*sizeof(FILE*));
	FILE** outStreams = (FILE**) malloc(10*sizeof(FILE*));

	int nbInStreams = 0;
	int nbOutStreams = 0;

	int MAX_SCHEDULE_COUNT = cfg.has("ms") ? std::stoi(cfg["ms"]) : -1;
	int MAX_PROC_COUNT = cfg.has("mp") ? std::stoi(cfg["mp"]) : -1;

	if (cfg.has("i"))
	{
		for (auto filename : cfg.argsOf("i"))
		{
			FILE* fp = ios[filename];
			if (!fp)
				fp = fopen(filename.c_str(), "r");
			inStreams[nbInStreams++] = fp;
		}
	}

	if (cfg.has("o"))
	{
		for (auto filename : cfg.argsOf("o"))
		{
			FILE* fp = ios[filename];
			if (!fp)
				fp = fopen(filename.c_str(), "w");
			outStreams[nbOutStreams++] = fp;
		}
	}


	int localArgc = 1;
	char ** localArgv;

	// the application has an argc equal to the number of options + 1 for its path
	// cfg stores all application arguments in the "--" option
	//
	// as neither the C nor the POSIX standard ensures that the arguments are stored
	// in contiguous space, we can use cfg's c_str() as pointers
	if (cfg.has("-"))
	{
		// we reserve localArgv's elements
		localArgc += cfg.argsOf("-").size();
		localArgv = new char*[localArgc];

		// the first argument is always the path
		localArgv[0] = binaryFile;

		int argId = 1;
		// we set localArgv values
		for (const std::string& arg : cfg.argsOf("-"))
		{
			localArgv[argId] = (char*)arg.c_str();
			argId++;
		}
	}
	else
	{
		localArgv = new char*[localArgc];
		localArgv[0] = binaryFile;
	}


	if (cfg.has("h") || binaryFile == NULL){
		Log::printf(0, "Usage is %s -f elfFile [options] -- args\n", argv[0]);
		Log::printf(0, "where elfFile is an RISCV64-imf elf file, args are the argument given to the simulated application and options are one of the following:\n");

		Log::printf(0, "\t-O n\t\tSet optimization level between 0 and 3.\n");
		Log::printf(0, "\t\t\t  Opt 0 means that instructions are only naively translated\n");
		Log::printf(0, "\t\t\t  Opt 1 means that basic blocks are scheduled\n");
		Log::printf(0, "\t\t\t  Opt 2 means that frequently executed procedure are built and optimized\n");
		Log::printf(0, "\t\t\t  Opt 3 means that different VLIW configuration are explored\n");
		Log::printf(0, "\t-c n\t\tSet the initial VLIW configuration to use.\n");

		Log::printf(0, "\t-i file\t\tAdding an input file to use as standard input for simulated application\n");
		Log::printf(0, "\t-o file\t\tAdding an output file to use as standard output for simulated application. Can add up to 10 files by repeating -o file\n");


		Log::printf(0, "\t-v n\t\tSet verbose mode to a level between 0 and 9. Higher level means more messages.\n");
		Log::printf(0, "\t-statmode n\tAllows to set stat mode to one in order to have parseable stats\n");

		return 1;
	}


	/***********************************
	 *  Initialization of the DBT platform
	 ***********************************
	 * In the linux implementation, this is done by reading an elf file and copying binary instructions
	 * in the corresponding memory.
	 * In a real platform, this may require no memory initialization as the binaries would already be stored in the
	 * system memory.
	 ************************************/

	//Definition of objects used for DBT process
	DBTPlateform dbtPlateform(MEMORY_SIZE);

	dbtPlateform.vliwInitialConfiguration = CONFIGURATION;
	dbtPlateform.vliwInitialIssueWidth = getIssueWidth(dbtPlateform.vliwInitialConfiguration);

	//Preparation of required memories
	for (int oneFreeRegister = 33; oneFreeRegister<63; oneFreeRegister++)
		dbtPlateform.freeRegisters[oneFreeRegister-33] = oneFreeRegister;

	for (int oneFreeRegister = 63-33; oneFreeRegister<63; oneFreeRegister++)
		dbtPlateform.freeRegisters[oneFreeRegister] = 0;

	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		dbtPlateform.placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;
	//same for FP registers
	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		dbtPlateform.placeOfRegisters[256+64+onePlaceOfRegister] = onePlaceOfRegister;



	#ifndef __NIOS

	ThreadedDebug * dbg = nullptr;
	TraceQueue * tracer = nullptr;
	if (cfg.has("trace")) {
		TraceQueue * tracer = new TraceQueue();

		dbg = new ThreadedDebug(tracer);
		dbg->run();

		dbtPlateform.vexSimulator = new VexTraceSimulator(dbtPlateform.vliwBinaries, tracer);
	}
	else
		dbtPlateform.vexSimulator = new VexCgraSimulator(dbtPlateform.vliwBinaries);

	dbtPlateform.vexSimulator->inStreams = inStreams;
	dbtPlateform.vexSimulator->nbInStreams = nbInStreams;
	dbtPlateform.vexSimulator->outStreams = outStreams;
	dbtPlateform.vexSimulator->nbOutStreams = nbOutStreams;

	setVLIWConfiguration(dbtPlateform.vexSimulator, dbtPlateform.vliwInitialConfiguration);

	#endif

	unsigned char* code;
	unsigned int addressStart;
	unsigned int size;
	unsigned int pcStart;


	//We read the binaries
	readSourceBinaries(binaryFile, code, addressStart, size, pcStart, &dbtPlateform);

	if (size > MEMORY_SIZE){
		Log::printf(LOG_ERROR, "ERROR: Size of source binaries is %d. Current implementation only accept size lower then %d\n", size, MEMORY_SIZE);
		exit(-1);
	}


	int numberOfSections = 1 + (size>>10);
	IRApplication application = IRApplication(numberOfSections);
	Profiler profiler = Profiler(&dbtPlateform);




	//we copy the binaries in the corresponding memory
	for (int oneInstruction = 0; oneInstruction<size; oneInstruction++)
		dbtPlateform.mipsBinaries[oneInstruction] = ((unsigned int*) code)[oneInstruction];


	//We declare the variable in charge of keeping a track of where we are writing
	unsigned int placeCode = 0; //As 4 instruction bundle

	//We add initialization code to the vliw binaries
	placeCode = getInitCode(&dbtPlateform, placeCode, addressStart);
	placeCode = insertCodeForInsertions(&dbtPlateform, placeCode, addressStart);

	//We modify the initialization call
	writeInt(dbtPlateform.vliwBinaries, 0*16, assembleIInstruction_sw(VEX_CALL, placeCode, 63));

	initializeInsertionsMemory(size*4);
//	for (int oneInsertion=0; oneInsertion<placeCode; oneInsertion++){
//			Log::fprintf(0, stderr, "insert;%d\n", oneInsertion);
//	}
	/********************************************************
	 * First part of DBT: generating the first pass translation of binaries
	 *******************************************************
	 * TODO: description
	 *
	 *
	 ********************************************************/

	for (int i=0; i<placeCode; i++){
		dbtPlateform.vexSimulator->typeInstr[i] = 3;
	}
	int endOfInitSection = placeCode;

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

		buildBasicControlFlow(&dbtPlateform, oneSection,addressStart, startAddressSource, oldPlaceCode, placeCode, &application, &profiler);

//		int** insertions = (int**) malloc(sizeof(int **));
//		int nbIns = getInsertionList(oneSection*1024, insertions);
//		for (int oneInsertion=0; oneInsertion<nbIns; oneInsertion++){
//				Log::fprintf(0, stderr, "insert;%d\n", (*insertions)[oneInsertion]+(*insertions)[-1]);
//		}
	}

	for (int oneUnresolvedJump = 0; oneUnresolvedJump<unresolvedJumpsArray[0]; oneUnresolvedJump++){
		unsigned int source = unresolvedJumpsSourceArray[oneUnresolvedJump+1];
		unsigned int initialDestination = unresolvedJumpsArray[oneUnresolvedJump+1];
		unsigned int type = unresolvedJumpsTypeArray[oneUnresolvedJump+1];

		unsigned char isAbsolute = ((type & 0x7f) != VEX_BR) && ((type & 0x7f) != VEX_BRF && (type & 0x7f) != VEX_BLTU) && ((type & 0x7f) != VEX_BGE && (type & 0x7f) != VEX_BGEU) && ((type & 0x7f) != VEX_BLT);
		unsigned int destinationInVLIWFromNewMethod = solveUnresolvedJump(&dbtPlateform, initialDestination);

		if (destinationInVLIWFromNewMethod == -1){
			Log::printf(LOG_ERROR, "A jump from %d to %x is still unresolved... (%d insertions)\n", source, initialDestination, insertionsArray[(initialDestination>>10)<<11]);
			exit(-1);
		}
		else{
			int immediateValue = (isAbsolute) ? (destinationInVLIWFromNewMethod) : ((destinationInVLIWFromNewMethod  - source));
			int mask = (isAbsolute) ? 0x7ffff : 0x1fff;

			writeInt(dbtPlateform.vliwBinaries, 16*(source), type + ((immediateValue & mask)<<7));

			if (immediateValue > 0x7ffff){
				Log::fprintf(LOG_ERROR, stderr, "error in immediate size...\n");
				exit(-1);
			}
			unsigned int instructionBeforePreviousDestination = readInt(dbtPlateform.vliwBinaries, 16*(destinationInVLIWFromNewMethod-1)+12);
			if (instructionBeforePreviousDestination != 0)
				writeInt(dbtPlateform.vliwBinaries, 16*(source+1)+12, instructionBeforePreviousDestination);
		}
	}

	for (int i=endOfInitSection; i<placeCode; i++){
		dbtPlateform.vexSimulator->typeInstr[i] = 0;
	}



	//We also add information on insertions
	int insertionSize = 65536;
	int areaCodeStart=1;
	int areaStartAddress = 0;

	#ifndef __NIOS
	dbtPlateform.vexSimulator->initializeDataMemory((unsigned char*) insertionsArray, 65536*4, 0x80000000);
	#endif

	#ifndef __NIOS
	dbtPlateform.vexSimulator->initializeRun(0, localArgc, localArgv);
	#endif

	//We change the init code to jump at the correct place
	unsigned int translatedStartPC = solveUnresolvedJump(&dbtPlateform, (pcStart-addressStart)>>2);
	unsigned int instruction = assembleIInstruction_sw(VEX_CALL, translatedStartPC, 63);
	writeInt(dbtPlateform.vliwBinaries, 0, instruction);




	int runStatus=0;
	int abortCounter = 0;

	float coef = 0;

	//We modelize the translation time

	//SW version
//	dbtPlateform.vexSimulator->cycle = 1000*425;
//	runStatus = run(&dbtPlateform, size*425);

	//HW version
	dbtPlateform.vexSimulator->cycle = 1000;
	runStatus = run(&dbtPlateform, size);

	while (runStatus == 0){



		int oldOptimizationCount = dbtPlateform.optimizationCycles;
		bool optimizationPerformed = false;

		if (OPTLEVEL >= 3){
			for (int oneProcedure = 0; oneProcedure < application.numberProcedures; oneProcedure++){
				IRProcedure *procedure = application.procedures[oneProcedure];
				if (procedure->state == 0){
					char oldPrevious = procedure->previousConfiguration;
					char oldConf = procedure->configuration;

					changeConfiguration(procedure);
					if (procedure->configuration != oldConf || procedure->configurationScores[procedure->configuration] == 0){
						IRProcedure *scheduledProcedure = rescheduleProcedure_schedule(&dbtPlateform, procedure, placeCode);
						suggestConfiguration(procedure, scheduledProcedure);

						int score = computeScore(scheduledProcedure);
						procedure->configurationScores[procedure->configuration] = score;

						if (score > procedure->configurationScores[procedure->previousConfiguration]){
							placeCode = rescheduleProcedure_commit(&dbtPlateform, procedure, placeCode, scheduledProcedure);
						}
						else{
							procedure->configuration = procedure->previousConfiguration;
							procedure->previousConfiguration = oldPrevious;
						}
					}
					optimizationPerformed = true;
					break;
				}
				else if (procedure->state==1){
					char maxConf = 0;
					int bestScore = 0;
					for (int oneConfiguration = 1; oneConfiguration<12; oneConfiguration++){
						char configuration = validConfigurations[oneConfiguration];
						float score = procedure->configurationScores[configuration];
						float energy = (score * getPowerConsumption(configuration))/10;
						if (procedure->configurationScores[configuration] > procedure->configurationScores[maxConf])
							if (-coef*energy + (1-coef)*score > bestScore){
								maxConf = configuration;
								bestScore = -coef*energy + (100-coef)*score;
							}

					}
					procedure->previousConfiguration = procedure->configuration;
					procedure->configuration = maxConf;
					placeCode = rescheduleProcedure(&dbtPlateform, procedure, placeCode);
					procedure->state = 2;
					dbtPlateform.procedureOptCounter++;
					optimizationPerformed = true;
				}
			}
		}

		for (int oneBlock = 0; oneBlock<profiler.getNumberProfiledBlocks(); oneBlock++){
			int profileResult = profiler.getProfilingInformation(oneBlock);
			IRBlock* block = profiler.getBlock(oneBlock);

			if ((MAX_PROC_COUNT==-1 || dbtPlateform.procedureOptCounter < MAX_PROC_COUNT) && block != NULL && OPTLEVEL >= 2 && profileResult >= 1 && (block->blockState == IRBLOCK_STATE_SCHEDULED || block->blockState == IRBLOCK_STATE_PROFILED)){

				int errorCode = buildAdvancedControlFlow(&dbtPlateform, block, &application);
				block->blockState = IRBLOCK_PROC;


				if (!errorCode){
					buildTraces(&dbtPlateform, application.procedures[application.numberProcedures-1]);

					placeCode = rescheduleProcedure(&dbtPlateform, application.procedures[application.numberProcedures-1], placeCode);
					dbtPlateform.procedureOptCounter++;

				}
				else{
				}


				optimizationPerformed = true;
				break;
			}

		}

		//We perform aggressive level 1 optimization: if a block takes more than 8 cycle we schedule it.
		//If it has a backward loop, we also profile it.
		for (int oneSection = 0; oneSection<numberOfSections; oneSection++){
			for (int oneBlock = 0; oneBlock<application.numbersBlockInSections[oneSection]; oneBlock++){
				IRBlock* block = application.blocksInSections[oneSection][oneBlock];

				if (block != NULL){
					if ((MAX_SCHEDULE_COUNT==-1 || dbtPlateform.blockScheduleCounter < MAX_SCHEDULE_COUNT) && OPTLEVEL >= 1 && block->sourceEndAddress - block->sourceStartAddress > 4  && block->blockState < IRBLOCK_STATE_SCHEDULED){


						optimizeBasicBlock(block, &dbtPlateform, &application, placeCode);
						dbtPlateform.blockScheduleCounter++;

						if ((block->sourceDestination != -1 && block->sourceDestination <= block->sourceStartAddress) || block->nbInstr > 32){
							profiler.profileBlock(block);
						}

						optimizationPerformed = true;
						break;
					}

				}

			}

			if (optimizationPerformed)
				break;

		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ARTHUR BEGIN
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if (OPTLEVEL >= 2 && cgraConfs < 2)
		{
			for (unsigned int procId = 0; procId < application.numberProcedures; ++procId)
			{
				IRProcedure * proc = application.procedures[procId];
				bool opt_performed = false;
				for (int blockId = 0; blockId < proc->nbBlock && cgraConfs < 2; ++blockId)
				{
					IRBlock * block = proc->blocks[blockId];
					localRegisterAnalysis(block);
					for (int succId = 0; succId < block->nbSucc; ++succId)
						if (block->successors[succId] == block)//(block->sourceDestination == block->sourceStartAddress)
						{
							// test de l'eligibilite d'un bloc
							// un bloc est eligible si ses instructions sont toutes implementables dans le CGRA
							bool eligible = true;
							uint128_struct * to_schedule = new uint128_struct[block->nbInstr];
							int instrId;
							for (instrId = 0; instrId < block->nbInstr; ++instrId)
							{
								to_schedule[instrId].word96 = readInt(block->instructions, instrId*16+0);
								to_schedule[instrId].word64 = readInt(block->instructions, instrId*16+4);
								to_schedule[instrId].word32 = readInt(block->instructions, instrId*16+8);
								to_schedule[instrId].word0 = readInt(block->instructions, instrId*16+12);

								if (!cgra::isEligible(to_schedule[instrId].word96
																,to_schedule[instrId].word64
																,to_schedule[instrId].word32
																,to_schedule[instrId].word0))
								{
									Log::out(0) << printBytecodeInstruction(instrId, to_schedule[instrId].word96, to_schedule[instrId].word64, to_schedule[instrId].word32, to_schedule[instrId].word0);
									Log::out(0) << "EST INVALIDE\n";
									break;
								}

								Log::out(0) << printBytecodeInstruction(instrId, to_schedule[instrId].word96, to_schedule[instrId].word64, to_schedule[instrId].word32, to_schedule[instrId].word0);
							}
							// si eligible, on le transforme en configuration CGRA
							if (instrId != 0)
							{
								// configuration du CGRA
								PcmWrapper::startRecord();
								VexCgraSimulator * sim = (dynamic_cast<VexCgraSimulator*>(dbtPlateform.vexSimulator));
								if (scheduler->schedule(sim->cgraSimulator, to_schedule, instrId))
								{
									++cgraConfs;
									opt_performed = true;
									//Log::out(2) << "Schedule of " << block << " ok !\n";
									int id;

//									FILE * f = fopen(std::string("/home/ablanleu/Documents/stage/xdot/cgra"+std::to_string(sim->configurationCache.size()-1)+".dot").c_str(), "w");
//									FILE * f2 = fopen(std::string("/home/ablanleu/Documents/stage/xdot/cgra"+std::to_string(sim->configurationCache.size()-1)+".txt").c_str(), "w");
//									IRBlock b(0,0,0);
//									b.instructions = block->instructions;
//									b.nbInstr = instrId;
//									b.print(f, f2);
//									fclose(f);
//									fclose(f2);

									//cgra::printConfig(0, sim->configurationCache[sim->configurationCache.size()-1].configuration);


									for (id = instrId; id < block->nbInstr; ++id)
									{
										uint128_struct instr;

										instr.word96 = readInt(block->instructions, id*16+0);
										instr.word64 = readInt(block->instructions, id*16+4);
										instr.word32 = readInt(block->instructions, id*16+8);
										instr.word0  = readInt(block->instructions, id*16+12);

										short virtualRDest = ((instr.word64>>14) & 0x1ff);
										short virtualRIn2 = ((instr.word64>>23) & 0x1ff);
										short virtualRIn1 = ((instr.word96>>0) & 0x1ff);

										virtualRDest = virtualRDest < instrId ? ((readInt(block->instructions, virtualRDest*16+4) >> 14) & 0x1ff)
																													: (virtualRDest >= 256 ? virtualRDest : virtualRDest - instrId + 1);

										virtualRIn2 = virtualRIn2 < instrId ? ((readInt(block->instructions, virtualRIn2*16+4) >> 14) & 0x1ff)
																												: (virtualRIn2 >= 256 ? virtualRIn2 : virtualRIn2 - instrId + 1);

										if (!((instr.word96 >> 18) & 0x1))
										virtualRIn1 = virtualRIn1 < instrId ? ((readInt(block->instructions, virtualRIn1*16+4) >> 14) & 0x1ff)
																												: (virtualRIn1 >= 256 ? virtualRIn1 : virtualRIn1 - instrId + 1);

										instr.word64 = (instr.word64 & 0b00000000000000000011111111111111) | (virtualRDest << 14) | (virtualRIn2 << 23);
										instr.word96 = (instr.word96 & 0b11111111111111111111111000000000) | (virtualRIn1);

										#define MINUS(word,off,val) (std::max(0, (((int)word >> off) & 0xff)-(val)) << off)
										instr.word32 = (instr.word32 & 0xff000000)
												+ MINUS(instr.word32,16,instrId-1)
												+ MINUS(instr.word32,8,instrId-1)
												+ MINUS(instr.word32,0,instrId-1);

										instr.word0 = MINUS(instr.word0,24,instrId-1)
												+ MINUS(instr.word0,16,instrId-1)
												+ MINUS(instr.word0,8,instrId-1)
												+ MINUS(instr.word0,0,instrId-1);

										writeInt(block->instructions, (id)*16+0, instr.word96);
										writeInt(block->instructions, (id)*16+4, instr.word64);
										writeInt(block->instructions, (id)*16+8, instr.word32);
										writeInt(block->instructions, (id)*16+12, instr.word0);
									}

									for (unsigned int jumpId = 0; jumpId < block->nbJumps; ++jumpId)
										block->jumpIds[jumpId] -= instrId - 1;

									writeInt(block->instructions, 0, to_schedule[0].word96);
									writeInt(block->instructions, 4, to_schedule[0].word64);
									writeInt(block->instructions, 8, to_schedule[0].word32);
									writeInt(block->instructions, 12, to_schedule[0].word0);

									block->nbInstr = block->nbInstr - instrId + 1;
									for (unsigned int id = 1; id < block->nbInstr; ++id)
									{
										writeInt(block->instructions, id*16+0, readInt(block->instructions, (id+instrId-1)*16+0));
										writeInt(block->instructions, id*16+4, readInt(block->instructions, (id+instrId-1)*16+4));
										writeInt(block->instructions, id*16+8, readInt(block->instructions, (id+instrId-1)*16+8));
										writeInt(block->instructions, id*16+12, readInt(block->instructions, (id+instrId-1)*16+12));
									}

//									for (instrId = 0; instrId < block->nbInstr; ++instrId)
//									{
//										Log::out(0) << printBytecodeInstruction(instrId,
//																														readInt(block->instructions, instrId*16+0),
//																														readInt(block->instructions, instrId*16+4),
//																														readInt(block->instructions, instrId*16+8),
//																														readInt(block->instructions, instrId*16+12));
//									}
									PcmWrapper::stopRecord();

									if (block->nbInstr == 1)
										block->nbJumps = 0;

									Log::out(0) << "CGRA Scheduled in " << PcmWrapper::getInstructionsRetired() << " instructions\n";
								}
							}
							delete[] to_schedule;
						}
				}

				if (opt_performed)
				{
					placeCode = rescheduleProcedure(&dbtPlateform, proc, placeCode);
				}
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ARTHUR END
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		int cyclesToRun = dbtPlateform.optimizationCycles - oldOptimizationCount;
		if (cyclesToRun == 0)
			cyclesToRun = 1000;

		runStatus = run(&dbtPlateform, cyclesToRun);


	}
	//We clean the last performance counters
	dbtPlateform.vexSimulator->timeInConfig[dbtPlateform.vexSimulator->currentConfig] += (dbtPlateform.vexSimulator->cycle - dbtPlateform.vexSimulator->lastReconf);

	Log::printStat(&dbtPlateform, &application);

	int nbFirstPass = 0;
	int nbScheduled = 0;
	int nbProc = 0;




	//We print profiling result
	#ifndef __NIOS
	delete dbtPlateform.vexSimulator;

	if (dbg)
		delete dbg;

	if (tracer)
		delete tracer;

	free(code);
	#endif

	delete localArgv;

	return 0;
}

