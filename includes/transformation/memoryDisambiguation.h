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
extern struct speculationDef speculationDefinitions[256];

#define PLSQ_BANK_SIZE 8

class MemoryDependencyGraph {
public:
  int size;
  unsigned char* idMem;
  char* idSpec;
  bool* graph;
  bool* isStore;

  MemoryDependencyGraph(IRBlock* block);
  ~MemoryDependencyGraph();

  void print();
  void transitiveReduction();
  void reduceArity();

  void applyGraph(IRBlock* block);
};

struct speculationDef {
  IRBlock* block;
  MemoryDependencyGraph* graph;
  unsigned char loads[PLSQ_BANK_SIZE];
  unsigned char stores[PLSQ_BANK_SIZE];
  short nbUse;
  short nbFail;
  char type, nbLoads, nbStores;
  char init;
};

void findAndInsertSpeculation(IRBlock* block, MemoryDependencyGraph* graph, IRBlock** predecessors, int nbPred);
void updateSpeculationsStatus(DBTPlateform* platform, int writePlace);

#endif /* INCLUDES_TRANSFORMATION_MEMORYDISAMBIGUATION_H_ */
