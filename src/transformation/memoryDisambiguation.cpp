/*
 * memoryDisambiguation.cpp
 *
 *  Created on: 19 déc. 2017
 *      Author: simon
 */

#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>
#include <isa/vexISA.h>
#include <lib/endianness.h>
#include <lib/log.h>
#include <transformation/memoryDisambiguation.h>

int MAX_DISAMB_COUNT = -1;

MemoryDependencyGraph::MemoryDependencyGraph(IRBlock *block){
	this->size = 0;
	this->idMem = (unsigned char*) malloc(block->nbInstr*sizeof(unsigned char));
	this->idSpec = (char*) malloc(block->nbInstr*sizeof(char));
	this->isStore = (bool*) malloc(block->nbInstr*sizeof(bool));

	for (int oneInstruction = 0; oneInstruction<block->nbInstr; oneInstruction++){
		int opcode = getOpcode(block->instructions, oneInstruction);
		if ((opcode >> 3) == (VEX_STW>>3) || (opcode >> 3) == (VEX_LDW>>3) || opcode == VEX_FSW || opcode == VEX_FLW){
			this->idMem[size] = oneInstruction;
			this->isStore[size] = (opcode >> 3) == (VEX_STW>>3) || opcode == VEX_FSW;
			this->idSpec[size] = -1;
			this->size++;
		}
	}

	this->graph = (bool*) malloc(this->size * this->size * sizeof(bool));

	for (int oneDep = 0; oneDep<this->size*this->size; oneDep++){
		this->graph[oneDep] = false;
	}

	for (int oneInstruction = 0; oneInstruction<this->size; oneInstruction++){
		int opcode = getOpcode(block->instructions, this->idMem[oneInstruction]);
		if ((opcode >> 3) == (VEX_STW>>3) || opcode == VEX_FSW){
			for (int oneOtherInstr = 0; oneOtherInstr<oneInstruction; oneOtherInstr++)
				this->graph[oneInstruction*this->size + oneOtherInstr] = true;
		}
		else if ((opcode >> 3) == (VEX_LDW>>3) || opcode == VEX_FLW){

			for (int oneOtherInstr = 0; oneOtherInstr<oneInstruction; oneOtherInstr++)
				if (this->isStore[oneOtherInstr])
					this->graph[oneInstruction*this->size + oneOtherInstr] = true;
		}
	}
}

MemoryDependencyGraph::~MemoryDependencyGraph(){
	free(this->graph);
	free(this->idMem);
	free(this->isStore);
}

void MemoryDependencyGraph::print(){
	Log::fprintf(LOG_MEMORY_DISAMBIGUATION, stderr, "    ");
	for (int oneInstr=0; oneInstr<this->size; oneInstr++){
		Log::fprintf(LOG_MEMORY_DISAMBIGUATION, stderr, "%3u ", (unsigned int) this->idMem[oneInstr]);
	}
	Log::fprintf(LOG_MEMORY_DISAMBIGUATION, stderr, "\n");
	for (int oneInstr=0; oneInstr<this->size; oneInstr++){
		Log::fprintf(LOG_MEMORY_DISAMBIGUATION, stderr, "%3d  ", this->idMem[oneInstr]);
		for (int oneOtherInstr = 0; oneOtherInstr<oneInstr; oneOtherInstr++){
			Log::fprintf(LOG_MEMORY_DISAMBIGUATION, stderr, "%d   ", graph[oneInstr*this->size + oneOtherInstr]?1:0);
		}
		Log::fprintf(LOG_MEMORY_DISAMBIGUATION, stderr, "\n");
	}
}

void MemoryDependencyGraph::transitiveReduction(){

	char *longestPaths = (char*) malloc(this->size * sizeof(longestPaths));

	for (int startPoint = 0; startPoint < this->size - 1; startPoint++){
		for (int oneDest = startPoint+1; oneDest<this->size; oneDest++){
			int longestPath = 0;

			if (graph[oneDest * this->size + startPoint]){
				longestPath = 1;
			}
			for (int onePred = startPoint+1; onePred<oneDest; onePred++){
				if (graph[oneDest * this->size + onePred] && longestPaths[onePred] != 0){
					longestPath = 2;
				}
			}

			if (longestPath == 2)
				graph[oneDest * this->size + startPoint] = false;

			longestPaths[oneDest] = longestPath;
		}
	}

}

void MemoryDependencyGraph::reduceArity(){

	//This function will reduce the arity of the first node having more than four pred
	for (int oneMemInstruction = 0; oneMemInstruction<this->size; oneMemInstruction++){
		int nbPred = 0;

		for (int onePredecessor = 0; onePredecessor<oneMemInstruction; onePredecessor++)
			if (this->graph[oneMemInstruction * this->size + onePredecessor])
				nbPred++;

		//If arity is greater than four we have to reduce it
		char lastPred[4];
		char nbLastPred = 0;
		char writeLastPred = 0;


		if (nbPred > 4){
			for (int onePredecessor = oneMemInstruction-1; onePredecessor>=0; onePredecessor--){
				if (this->graph[oneMemInstruction * this->size + onePredecessor]){
					if (nbLastPred<4){
						lastPred[writeLastPred] = onePredecessor;
						nbLastPred++;
						writeLastPred = (writeLastPred + 1) % 4;
					}
					else{
						graph[lastPred[writeLastPred] * this->size + onePredecessor] = true;
						graph[oneMemInstruction * this->size + onePredecessor] = false;
						lastPred[writeLastPred] = onePredecessor;
						writeLastPred = (writeLastPred + 1) % 4;
					}

				}

			}
		}

	}

}

void MemoryDependencyGraph::applyGraph(IRBlock *block){

	//We first reduce the graph by applying transitive reduction
	this->transitiveReduction();


	//Then we reduce the arity to be compliant with IR constraints
	this->reduceArity();
	this->print();

	//We remove previous dependencies
	for (int oneMemoryInstr = 0; oneMemoryInstr<this->size; oneMemoryInstr++){
		unsigned char pred[7];
		char nbPred = getControlDep(block->instructions, this->idMem[oneMemoryInstr], pred);
		clearControlDep(block->instructions, this->idMem[oneMemoryInstr]);

		for (int onePred = 0; onePred < nbPred; onePred++){
			char opcode = getOpcode(block->instructions, pred[onePred]);
			if (!((opcode >> 4) == 1 || opcode == VEX_FLW || opcode == VEX_FSW)){
				addControlDep(block->instructions, pred[onePred], this->idMem[oneMemoryInstr]);
			}
		}

	}

	//We add dependencies when needed
	for (int oneMemoryInstr = 0; oneMemoryInstr<this->size; oneMemoryInstr++)
		for (int oneOtherMemoryInstr = 0; oneOtherMemoryInstr<oneMemoryInstr; oneOtherMemoryInstr++)
			if (graph[oneMemoryInstr*this->size + oneOtherMemoryInstr])
				addControlDep(block->instructions, this->idMem[oneOtherMemoryInstr], this->idMem[oneMemoryInstr]);

}

void basicMemorySimplification(IRBlock *block, MemoryDependencyGraph *graph){

	for (int oneMemInstruction = 0; oneMemInstruction<graph->size; oneMemInstruction++){
		int memInstruction = graph->idMem[oneMemInstruction];

		for (int onePredecessor = 0; onePredecessor<oneMemInstruction; onePredecessor++){
			int predecessor = graph->idMem[onePredecessor];
			if (graph->graph[oneMemInstruction * graph->size + onePredecessor]){
				short operandsPred[2], operandsInstr[2];
				getOperands(block->instructions, predecessor, operandsPred);
				getOperands(block->instructions, memInstruction, operandsInstr);

				int immediateValuePred, immediateValueInstr;
				getImmediateValue(block->instructions, predecessor, &immediateValuePred);
				getImmediateValue(block->instructions, memInstruction, &immediateValueInstr);

				if (operandsInstr[0] == operandsPred[0] && immediateValueInstr != immediateValuePred){
					graph->graph[oneMemInstruction * graph->size + onePredecessor] = false;
				}
			}
		}
	}
}

void findAndInsertSpeculation(IRBlock *block, MemoryDependencyGraph *graph){
	//A good candidate is a list of loads which all depends on a given set of stores
fprintf(stderr, "test\n");

	char currentSpecId = 0;

	//We find a store with an ID strictly greater than 0
	int index = 0;
	bool found = false;
	while (!found && index < graph->size){
		fprintf(stderr, "test %d\n", index);

		if (graph->isStore[index] && graph->idMem[index] > 0){
			found = true;
		}
		index++;
	}

	if (found){

		//A store has been found, we will go through all memory accesses of the block until we went across 4 load instructions.
		//All the memory accesses that have been met will be done speculatively (both loads and stores)
		// if we reach 4 loads, the speculation area will be terminated and we will start a new speculation area
		int nbLoads = 0;
		int storeIndex = index-1;
		bool isSpec = false;

		while (index < graph->size && nbLoads < 4){

			int imm = 0;
			bool hasImm = getImmediateValue(block->instructions, graph->idMem[index], &imm);

			fprintf(stderr, "while trying to spec, imm is %x\n", imm);
			if (imm > 64 || imm <= -64){
				fprintf(stderr, "Failed at reducing immediate\n");
				exit(-1);
			}
			else if (imm != 0){

				fprintf(stderr, "setting imm at %x\n", imm<<5);
				setImmediateValue(block->instructions, graph->idMem[index], imm<<5);
			}


			if (graph->isStore[index])
				block->instructions[4*graph->idMem[index]] |= 0x20 | (currentSpecId<<1) | 1;
			else
				block->instructions[4*graph->idMem[index]] |= (currentSpecId<<1) | 1;

			graph->idSpec[index] = currentSpecId;

			for (int onePreviousInstr = storeIndex; onePreviousInstr<index; onePreviousInstr++){
				graph->graph[index*graph->size + onePreviousInstr] = false;
			}

			if (!graph->isStore[index])
				nbLoads++;

			isSpec = true;
			index++;
		}

		if (isSpec){
			int imm = 0;
			bool hasImm = getImmediateValue(block->instructions, graph->idMem[storeIndex], &imm);
			if (imm > 64){
				fprintf(stderr, "Failed at reducing immediate\n");
				exit(-1);
			}
			else if (imm != 0){
				setImmediateValue(block->instructions, graph->idMem[storeIndex], imm<<5);
			}

			block->instructions[4*graph->idMem[storeIndex]] |= 0x20 | (currentSpecId<<1) | 1;
			graph->idSpec[storeIndex] = currentSpecId;
		}
	}

}


void memoryDisambiguation(DBTPlateform *platform, IRBlock *block){
		MemoryDependencyGraph *graph = new MemoryDependencyGraph(block);

		graph->print();


		// We print debug
		Log::printf(LOG_MEMORY_DISAMBIGUATION, "************************************************************\n");
		Log::printf(LOG_MEMORY_DISAMBIGUATION, "*****             Memory disambiguation process       ******\n");
		Log::printf(LOG_MEMORY_DISAMBIGUATION, "************************************************************\n");
		Log::printf(LOG_MEMORY_DISAMBIGUATION, "Before disambiguation: \n");
		for (int i=0; i<block->nbInstr; i++)
			Log::printf(LOG_MEMORY_DISAMBIGUATION, "%s ", printBytecodeInstruction(i, readInt(block->instructions, i*16+0), readInt(block->instructions, i*16+4), readInt(block->instructions, i*16+8), readInt(block->instructions, i*16+12)).c_str());

		//We perform disambiguation and apply it
		basicMemorySimplification(block, graph);
		findAndInsertSpeculation(block, graph);

		graph->applyGraph(block);

		//We print debug
		Log::printf(LOG_MEMORY_DISAMBIGUATION, "\n After disambiguation: \n");
		for (int i=0; i<block->nbInstr; i++)
			Log::printf(LOG_MEMORY_DISAMBIGUATION, "%s ", printBytecodeInstruction(i, readInt(block->instructions, i*16+0), readInt(block->instructions, i*16+4), readInt(block->instructions, i*16+8), readInt(block->instructions, i*16+12)).c_str());

		Log::printf(LOG_MEMORY_DISAMBIGUATION, "************************************************************\n");

		//We cleanout
		delete graph;
}
