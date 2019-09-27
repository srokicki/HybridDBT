#include <isa/vexISA.h>
#include <isa/irISA.h>
#include <lib/log.h>
#include <lib/endianness.h>
#include <dbt/dbtPlateform.h>

#define NB_REG 512
#define SEW_SIZE 16

//Global variable counterMeasureType describe the type of spectre countermeasure applied
//	- lower bit represents the countermeasure activation for branch speculation
//  - second lower bit represents the countermeasure activation for memory dependency speculation
//  - next bits represent the type of mitigation : 0 = fine-grained, 1 = fence, 2 = no spec (TODO)

char counterMeasureType;

void checkSpeculation(IRProcedure* procedure){

	if (counterMeasureType){
		Log::printf(LOG_CHECK_SPECULATION, "checkSpeculation\n");
		for (unsigned i_trace = 0; i_trace < procedure->nbBlock; i_trace++){
			IRBlock* trace = procedure->blocks[i_trace];

			Log::printf(LOG_CHECK_SPECULATION, "Checking speculation in trace:\n");
			for (int i=0; i<trace->nbInstr; i++){
			  Log::printf(LOG_CHECK_SPECULATION, "%s ", printBytecodeInstruction(i, readInt(trace->instructions, i*16+0), readInt(trace->instructions, i*16+4), readInt(trace->instructions, i*16+8), readInt(trace->instructions, i*16+12)).c_str());
			}
			Log::printf(LOG_CHECK_SPECULATION, "\n");


			for (unsigned i_br = 0; i_br < trace->nbInstr; i_br++){
				char br_opcode = getOpcode(trace->instructions, i_br);


				/***********************************************************************************************************
				 *********  Trying to find the starting element:
				 *********  It may be a branch instruction or a speculative store
				 ***********************************************************************************************************/

				bool isBranch = (counterMeasureType&0x1) && (br_opcode == VEX_BR || br_opcode == VEX_BRF || br_opcode == VEX_BLT || br_opcode == VEX_BGE || br_opcode == VEX_BLTU || br_opcode == VEX_BGEU);
				bool isSpeculativeStore = (counterMeasureType&0x2) && ((br_opcode == VEX_STB || br_opcode == VEX_STH || br_opcode == VEX_STW || br_opcode == VEX_STD)
						&& (trace->instructions[4*i_br] & 0x1));

				if (isBranch || isSpeculativeStore){

					if (isBranch)
						fprintf(stderr, "For br %d\n", i_br);
					else
						fprintf(stderr, "For spec store %d\n", i_br);

					//Initialization of all tainting registers at zero
					int taint[NB_REG] = { 0 };


					//We check if we have to apply the countermeasure in the current speculation area
					//According to the type of the countermeasure, there may be different condition
					//For example, the no speculation countermeasure always apply it
					bool applyCounterMeasureEverywhere = false;

					if ((counterMeasureType>>2) == 2)
						applyCounterMeasureEverywhere = true;

					for (int i_instr = i_br + 1; i_instr < trace->nbInstr; i_instr ++){

						char instr_opcode = getOpcode(trace->instructions, i_instr);

						bool isBranch2 = (counterMeasureType&0x1) && (instr_opcode == VEX_BR || instr_opcode == VEX_BRF || instr_opcode == VEX_BLT || instr_opcode == VEX_BGE || instr_opcode == VEX_BLTU || instr_opcode == VEX_BGEU);
						bool isSpeculativeStore2 = (counterMeasureType&0x2) && ((instr_opcode == VEX_STB || instr_opcode == VEX_STH || instr_opcode == VEX_STW || instr_opcode == VEX_STD)
								&& (trace->instructions[4*i_instr] & 0x1));

						//Solution for early escape
						if (isBranch2 || isSpeculativeStore2)
							break;

						short operands[] = {0,0};
						char nb_operands = getOperands(trace->instructions, i_instr, operands);

						//We check if operands are tainted
						for (unsigned oneOperand = 0; oneOperand < nb_operands; oneOperand++){

							if (operands[oneOperand] < 256)
								operands[oneOperand] = getDestinationRegister(trace->instructions, operands[oneOperand]);

							if (taint[operands[oneOperand]]){ //if operand is tainted

								short instr_destreg = getDestinationRegister(trace->instructions, i_instr);
								if (instr_destreg >= 0) 		//if there is a dest
									taint[instr_destreg] = 1; 	//we taint the destination register


								if (instr_opcode >= VEX_LDD && instr_opcode < VEX_PROFILE){ //if instr is a load (has an impact on something)

									if ((counterMeasureType >> 2) == 0){
										fprintf(stderr, "Adding dep\n");
										addControlDep(trace->instructions, i_br, i_instr);
									}

									if ((counterMeasureType >> 2) == 1)
										applyCounterMeasureEverywhere = true;

								}
							}
						}

						//We taint the destination
						if (instr_opcode >= VEX_LDD && instr_opcode < VEX_PROFILE){ // if instruction is a load after a branch
							short instr_destreg = getDestinationRegister(trace->instructions, i_instr);

							if (instr_destreg >= 0){ //if we have a dest
								taint[instr_destreg] = 1; //the dest is tainted
							}
						}
					}

					//If we need to apply the countermeasure everywhere, we do it
					if (applyCounterMeasureEverywhere){
						for (int i_instr = i_br + 1; i_instr < trace->nbInstr; i_instr ++){
							addControlDep(trace->instructions, i_br, i_instr);
							fprintf(stderr, "Adding dep\n");

						}
					}



				}
			}

			Log::printf(LOG_CHECK_SPECULATION, "New trace:\n");
			for (int i=0; i<trace->nbInstr; i++){
			  Log::printf(LOG_CHECK_SPECULATION, "%s ", printBytecodeInstruction(i, readInt(trace->instructions, i*16+0), readInt(trace->instructions, i*16+4), readInt(trace->instructions, i*16+8), readInt(trace->instructions, i*16+12)).c_str());
			}
			Log::printf(LOG_CHECK_SPECULATION, "\n");

		}
    }
}
