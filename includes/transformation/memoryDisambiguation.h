/*
 * memoryDisambiguation.h
 *
 *  Created on: 22 févr. 2018
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_
#define INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_


class MemoryDependencyGraph{
public:
	int size;
	char *idMem;
	bool *graph;
	bool *isStore;

	MemoryDependencyGraph(IRBlock *block);
	~MemoryDependencyGraph();

	void print();
	void transitiveReduction();
	void applyGraph(IRBlock *block);

};



#endif /* INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_ */
