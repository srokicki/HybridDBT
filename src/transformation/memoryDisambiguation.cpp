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

MemoryDependencyGraph::MemoryDependencyGraph(IRBlock *block){
	this->size = 0;

	for (int oneInstruction = 0; oneInstruction<block->nbInstr; oneInstruction++){
		int opcode = getOpcode(block->instructions, oneInstruction);
		if ((opcode >> 3) == (VEX_STW>>3) || (opcode >> 3) == (VEX_LDW>>3) || opcode == VEX_FSW || opcode == VEX_FLW)
			this->size++;
	}

	this->idMem = (char*) malloc(this->size*sizeof(char));
	this->isStore = (bool*) malloc(this->size*sizeof(bool));
	this->graph = (bool*) malloc(this->size * this->size * sizeof(bool));

	for (int oneDep = 0; oneDep<this->size*this->size; oneDep++){
		this->graph[oneDep] = false;
	}

	for (int oneInstruction = 0; oneInstruction<block->nbInstr; oneInstruction++){
		int opcode = getOpcode(block->instructions, oneInstruction);
		if ((opcode >> 3) == (VEX_STW>>3) || opcode == VEX_FSW){
			this->idMem[this->size] = oneInstruction;
			this->isStore[this->size] = true;
			for (int oneOtherInstr = 0; oneOtherInstr<this->size; oneOtherInstr++)
				this->graph[oneOtherInstr*this->size] = true;


			clearControlDep(block->instructions, oneInstruction);
		}
		else if ((opcode >> 3) == (VEX_LDW>>3) || opcode == VEX_FLW){
			this->idMem[size] = oneInstruction;
			this->isStore[size] = false;
			for (int oneOtherInstr = 0; oneOtherInstr<this->size; oneOtherInstr++)
				if (this->isStore[oneOtherInstr])
					this->graph[oneOtherInstr*this->size] = true;


			clearControlDep(block->instructions, oneInstruction);
		}
	}
}

MemoryDependencyGraph::~MemoryDependencyGraph(){
	free(this->graph);
	free(this->idMem);
	free(this->isStore);
}

void MemoryDependencyGraph::print(){
	fprintf(stderr, "size is %d\n", size);
	for (int oneInstr=0; oneInstr<this->size; oneInstr++){
		fprintf(stderr, "%2d ", this->idMem[oneInstr]);
	}
	fprintf(stderr, "\n");
	for (int oneInstr=0; oneInstr<this->size; oneInstr++){
		fprintf(stderr, "%3d ", this->idMem[oneInstr]);
		for (int oneOtherInstr = 0; oneOtherInstr<oneInstr; oneOtherInstr++){
			fprintf(stderr, " %d  ", graph[oneInstr*this->size + oneOtherInstr]?1:0);
		}
		fprintf(stderr, "\n");
	}
}

void MemoryDependencyGraph::transitiveReduction(){

}

void MemoryDependencyGraph::applyGraph(IRBlock *block){

}

void memoryDisambiguation(DBTPlateform *platform, IRBlock *block){
	MemoryDependencyGraph *graph = new MemoryDependencyGraph(block);

	graph->print();

	delete graph;
}
