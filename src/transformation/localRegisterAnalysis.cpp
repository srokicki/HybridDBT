#include <transformation/localRegisterAnalysis.h>

#include <vector>
#include <algorithm>
#include <lib/log.h>

//void _localRegisterAnalysis(IRBlock * block, uint8_t reg_state[64], std::vector<IRBlock *>& passed);

void localRegisterAnalysis(IRBlock * block)
{

	std::vector<IRBlock*> passed;
	passed.push_back(block);

	// determining which registers are being written by the analyzed block
	int16_t reg_state[64] = { -1 };

	for (unsigned int theId = 0; theId < block->nbInstr; ++theId)
	{
		short virtualRDest = getDestinationRegister(block->instructions, theId);
		if (virtualRDest >= 256)
		{
			reg_state[virtualRDest - 256] = theId;
			setAlloc(block->instructions, theId, 1);
		}
	}

	for (unsigned int i = 0; i < 64; ++i)
	{
		if (reg_state[i] >= 0)
			setAlloc(block->instructions, reg_state[i], 0);
	}
/*
	// analyse CFG to find if a register's value stays local or not
	for (unsigned int i = 0; i < block->nbSucc; ++i)
	{
		_localRegisterAnalysis(block->successors[i], reg_state, passed);
	}

	// set alloc=1 for each instruction which can write to a "local" register
	for (unsigned int theId = 0; theId < block->nbInstr; ++theId)
	{
		short virtualRDest = getDestinationRegister(block->instructions, theId);
		if (virtualRDest >= 256 && reg_state[virtualRDest-256])
		{
			setAlloc(block->instructions, theId, 1);
		}
	}
	*/
}
/*
void _localRegisterAnalysis(IRBlock * block, uint8_t reg_state[64], std::vector<IRBlock *> &passed)
{
	// avoid looping over same blocks
	if (std::find(passed.begin(), passed.end(), block) != passed.end())
		return;

	passed.push_back(block);

	// for each instruction of the current block
	for (unsigned int theId = 0; theId < block->nbInstr; ++theId)
	{
		short ops[2];
		short dest = getDestinationRegister(block->instructions, theId);
		char nb_ops = getOperands(block->instructions, theId, ops);

		// if one operand reads a register which has not been written yet
		// then the corresponding register is global
		for (unsigned int j = 0; j < nb_ops; ++j)
		{
			if (ops[j] >= 256 && reg_state[ops[j]-256] == 2)
				reg_state[ops[j]-256] = 0;
		}

		// if the instruction writes to a register which has not been read yet
		// then the register is local
		if (dest >= 256 && reg_state[dest-256] == 2)
			reg_state[dest-256] = 1;
	}

	// iterate over each successors
	for (unsigned int i = 0; i < block->nbSucc; ++i)
	{
		_localRegisterAnalysis(block->successors[i], reg_state, passed);
	}
}
*/
