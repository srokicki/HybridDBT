#include <isa/vexISA.h>
#include <isa/irISA.h>
#include <lib/log.h>
#include <lib/endianness.h>

#define NB_REG 512
#define SEW_SIZE 16

void checkSpeculation(IRProcedure* procedure){

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

			if (br_opcode == VEX_BR || br_opcode == VEX_BRF || br_opcode == VEX_BLT || br_opcode == VEX_BGE || br_opcode == VEX_BLTU || br_opcode == VEX_BGEU ){
				fprintf(stderr, "For br %d\n", i_br);

				int taint[NB_REG] = { 0 };

				for (unsigned i_instr = i_br + 1; i_instr < trace->nbInstr; i_instr ++){

					char instr_opcode = getOpcode(trace->instructions, i_instr);
					short operands[] = {0,0};
					char nb_operands = getOperands(trace->instructions, i_instr, operands);

					//We check if operands are tainted
					for (unsigned oneOperand = 0; oneOperand < nb_operands; oneOperand++)
					{
						if (operands[oneOperand] < 256)
							operands[oneOperand] = getDestinationRegister(trace->instructions, operands[oneOperand]);

						if (taint[operands[oneOperand]]) //if operand is tainted
						{
							short instr_destreg = getDestinationRegister(trace->instructions, i_instr);
							if (instr_destreg >= 0) //if there is a dest
								taint[instr_destreg] = 1; //we taint the destination register

							if (instr_opcode >= VEX_LDD && instr_opcode < VEX_PROFILE){ //if instr is a load (has an impact on something)
								addControlDep(trace->instructions, i_br, i_instr); //we add a control dep between branch and intr
								Log::printf(LOG_CHECK_SPECULATION, "Found a dep to add to %d\n", i_instr);
							}
						}
					}

					//We taint the destination
					if (instr_opcode >= VEX_LDD && instr_opcode < VEX_PROFILE){ // if instruction is a load after a branch
						short instr_destreg = getDestinationRegister(trace->instructions, i_instr);

						fprintf(stderr, "Found a load\n");
						if (instr_destreg >= 0){ //if we have a dest
							taint[instr_destreg] = 1; //the dest is tainted
							fprintf(stderr, "Tainting %d\n", instr_destreg);
						}
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
