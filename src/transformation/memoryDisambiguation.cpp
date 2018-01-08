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

void memoryDisambiguation(DBTPlateform *platform, IRBlock *block){

	char *idMem = (char*) malloc(block->nbInstr*sizeof(char));
	bool *isStore = (bool*) malloc(block->nbInstr*sizeof(bool));
	bool *deps = (bool*) malloc(block->nbInstr * block->nbInstr * sizeof(bool));

	for (int oneDep = 0; oneDep<block->nbInstr*block->nbInstr; oneDep++){
		deps[oneDep] = false;
	}

	for (int i=0; i<block->nbInstr; i++){
		Log::printf(LOG_SCHEDULE_PROC, "%s ", printBytecodeInstruction(i, readInt(block->instructions, i*16+0), readInt(block->instructions, i*16+4), readInt(block->instructions, i*16+8), readInt(block->instructions, i*16+12)).c_str());
	}
	int nbMem = 0;
	for (int oneInstruction = 0; oneInstruction<block->nbInstr; oneInstruction++){
		int opcode = getOpcode(block->instructions, oneInstruction);
		if ((opcode >> 3) == (VEX_STW>>3)){
			idMem[nbMem] = oneInstruction;
			isStore[nbMem] = true;
			for (int oneOtherInstr = 0; oneOtherInstr<nbMem; oneOtherInstr++){
				deps[oneOtherInstr*block->nbInstr] = true;
			}

			unsigned int bytecodeWord0 = readInt(block->instructions, oneInstruction*16+12);
			unsigned int bytecodeWord32 = readInt(block->instructions, oneInstruction*16+8);
			unsigned int bytecodeWord64 = readInt(block->instructions, oneInstruction*16+4);

			char nbDSucc = ((bytecodeWord64>>3) & 7);
			char nbSucc = ((bytecodeWord64>>0) & 7);
			char nbCSucc = nbSucc - nbDSucc;

			char newNbCSucc = 0;

			for (int oneControlSuccessor = 0; oneControlSuccessor < nbCSucc; oneControlSuccessor++){
				int predId;
				if (oneControlSuccessor>4){
					predId = (bytecodeWord32 >> ((oneControlSuccessor-4) * 8)) & 0xff;;

					fprintf(stderr, "instr %d has %d as pred \n", oneInstruction, predId);

					//					writeInt(bytecode, successor*16+8, bytecodeWord32);
				}
				else{
					predId = (bytecodeWord0 >> (oneControlSuccessor * 8)) & 0xff;
					fprintf(stderr, "instr %d has %d as pred \n", oneInstruction, predId);

//					writeInt(bytecode, successor*16+12, bytecodeWord0);
				}

				char predOpcode = getOpcode(block->instructions, predId);
				if ((predOpcode >> 4) != (VEX_LDW>>4)){
					if (oneControlSuccessor != newNbCSucc){
						if (newNbCSucc>3)
							bytecodeWord32 |= (predId<<((nbCSucc-4)*8));
						else
							bytecodeWord0 |= (predId<<((nbCSucc)*8));

					}
					newNbCSucc++;
				}
			}
			bytecodeWord64 = (bytecodeWord64 & 0xfffffff8) | (nbDSucc + newNbCSucc);

			writeInt(block->instructions, oneInstruction*16+12,bytecodeWord0);
			writeInt(block->instructions, oneInstruction*16+8, bytecodeWord32);
			writeInt(block->instructions, oneInstruction*16+4, bytecodeWord64);

			nbMem++;
		}
		else if ((opcode >> 3) == (VEX_LDW>>3)){
			idMem[nbMem] = oneInstruction;
			isStore[nbMem] = false;
			for (int oneOtherInstr = 0; oneOtherInstr<nbMem; oneOtherInstr++){
				if (isStore[oneOtherInstr])
					deps[oneOtherInstr*block->nbInstr] = true;
			}

			unsigned int bytecodeWord0 = readInt(block->instructions, oneInstruction*16+12);
						unsigned int bytecodeWord32 = readInt(block->instructions, oneInstruction*16+8);
						unsigned int bytecodeWord64 = readInt(block->instructions, oneInstruction*16+4);

						char nbDSucc = ((bytecodeWord64>>3) & 7);
						char nbSucc = ((bytecodeWord64>>0) & 7);
						char nbCSucc = nbSucc - nbDSucc;

						char newNbCSucc = 0;

						for (int oneControlSuccessor = 0; oneControlSuccessor < nbCSucc; oneControlSuccessor++){
							int predId;
							if (oneControlSuccessor>4){
								predId = (bytecodeWord32 >> ((oneControlSuccessor-4) * 8)) & 0xff;;

								fprintf(stderr, "instr %d has %d as pred \n", oneInstruction, predId);

								//					writeInt(bytecode, successor*16+8, bytecodeWord32);
							}
							else{
								predId = (bytecodeWord0 >> (oneControlSuccessor * 8)) & 0xff;
								fprintf(stderr, "instr %d has %d as pred \n", oneInstruction, predId);

			//					writeInt(bytecode, successor*16+12, bytecodeWord0);
							}

							char predOpcode = getOpcode(block->instructions, predId);
							if ((predOpcode >> 4) != (VEX_LDW>>4)){
								if (oneControlSuccessor != newNbCSucc){
									if (newNbCSucc>3)
										bytecodeWord32 |= (predId<<((nbCSucc-4)*8));
									else
										bytecodeWord0 |= (predId<<((nbCSucc)*8));

								}
								newNbCSucc++;
							}
						}
						bytecodeWord64 = (bytecodeWord64 & 0xfffffff8) | (nbDSucc + newNbCSucc);

						writeInt(block->instructions, oneInstruction*16+12,bytecodeWord0);
						writeInt(block->instructions, oneInstruction*16+8, bytecodeWord32);
						writeInt(block->instructions, oneInstruction*16+4, bytecodeWord64);




			nbMem++;
		}
	}
	for (int oneInstr=0; oneInstr<nbMem; oneInstr++){
		fprintf(stderr, "%2d ", idMem[oneInstr]);
	}
	fprintf(stderr, "\n");
	for (int oneInstr=0; oneInstr<nbMem; oneInstr++){
		fprintf(stderr, "%3d ", idMem[oneInstr]);
		for (int oneOtherInstr = 0; oneOtherInstr<oneInstr; oneOtherInstr++){
			fprintf(stderr, " %d  ", deps[oneInstr*block->nbInstr + oneOtherInstr]?1:0);
		}
		fprintf(stderr, "\n");
	}

	for (int i=0; i<block->nbInstr; i++){
		Log::printf(LOG_SCHEDULE_PROC, "%s ", printBytecodeInstruction(i, readInt(block->instructions, i*16+0), readInt(block->instructions, i*16+4), readInt(block->instructions, i*16+8), readInt(block->instructions, i*16+12)).c_str());
	}


	exit(-1);
}
