/*
 * memoryDisambiguation.h
 *
 *  Created on: 22 févr. 2018
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_
#define INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_

#include <isa/irISA.h>
#include <dbt/dbtPlateform.h>


extern int MAX_DISAMB_COUNT;
extern unsigned int speculationCounter;
extern struct speculationDef speculationDefinitions[256];

#define PLSQ_BANK_SIZE 8


class MemoryDependencyGraph{
public:
	unsigned int size;
	unsigned char *idMem;
	unsigned char *idSpec;
	bool *graph;
	bool *isStore;

	MemoryDependencyGraph(IRBlock *block);
	~MemoryDependencyGraph();

	void print();
	void transitiveReduction();
	void reduceArity();

	void applyGraph(IRBlock *block);

};

struct speculationDef {
	IRBlock *block;
	MemoryDependencyGraph *graph;
	unsigned char loads[PLSQ_BANK_SIZE];
	unsigned char stores[PLSQ_BANK_SIZE];
	unsigned short nbUse;
	unsigned short nbFail;
	unsigned char type, nbLoads, nbStores;
	unsigned char init;

};


void findAndInsertSpeculation(IRBlock *block, MemoryDependencyGraph *graph, IRBlock **predecessors, unsigned int nbPred);
void updateSpeculationsStatus(DBTPlateform *platform, int writePlace);


#endif /* INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_ */
