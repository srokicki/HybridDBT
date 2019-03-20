#include <isa/vexISA.h>
#include <isa/irISA.h>

#define NB_REG 512

void checkSpeculation(IRProcedure* procedure){

    for (unsigned i_trace = 0; i_trace < procedure->nbBlock; i_trace++){
		IRBlock* trace = procedure->blocks[i_trace];

		for (unsigned i_br = 0; i_br < trace->nbInstr; i_br++){
			char br_opcode = getOpcode(trace->instructions, i_br);

			if (br_opcode >= VEX_GOTO && br_opcode <= VEX_BGEU){


				int taint[NB_REG] = { 0 };

				for (unsigned i_instr = i_br + 1; i_instr < i_br + SEW && i_instr < trace->nbInstr; i_instr ++){  //SEW ??
					char instr_opcode = getOpcode(trace->instructions, i_instr);
					if (instr_opcode >= VEX_LDD && instr_opcode <= VEX_PROFILE){ // if instruction is a load after a branch
						short instr_destreg = getDestinationRegister(trace->instructions, i_instr);

						if (instr_destreg >= 0) //if we have a dest
							taint[instr_destreg] = 1; //the dest is tainted
					}

					short operands = {0,0};
					char nb_operands = getOperands(trace->instructions, i_instr, operands);

					for (unsigned oneOperand = 0; oneOperand < nb_operands; oneOperand++)
					{
						if (taint[operands[oneOperand]]) //if operand is tainted
						{
							short instr_destreg = getDestinationRegister(trace->instructions, i_instr);
							if (instr_destreg >= 0) //if there is a dest
								taint[instr_destreg] = 1; //we taint the destination register

							if (instr_opcode >= VEX_LDD && instr_opcode <= VEX_PROFILE) //if instr is a load (has an impact on something)
								addControlDep(trace->instructions, i_br, i_instr); //we add a control dep between branch and intr
						}
					}
				}
			}
		}
    }
}
