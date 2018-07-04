/*
 * memoryDisambiguation.h
 *
 *  Created on: 22 févr. 2018
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_
#define INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_

extern int MAX_DISAMB_COUNT;
extern unsigned char speculationCounter;

class MemoryDependencyGraph{
public:
	int size;
	unsigned char *idMem;
	char *idSpec;
	bool *graph;
	bool *isStore;

	MemoryDependencyGraph(IRBlock *block);
	~MemoryDependencyGraph();

	void print();
	void transitiveReduction();
	void reduceArity();

	void applyGraph(IRBlock *block);

};



#endif /* INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_ */
