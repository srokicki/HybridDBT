#include <transformation/cgraScheduler.h>
#include <lib/log.h>
#include <cstdio>

#include <vector>
#include <algorithm>


void printGraph(uint128_struct * instructions, uint32_t numInstructions);

CgraScheduler::CgraScheduler()
{

}

void CgraScheduler::schedule(CgraSimulator& cgra, uint128_struct * instructions, uint32_t numInstructions)
{
	printGraph(instructions, numInstructions);

	// place & route
	uint32_t configuration[4][3];

	for (uint32_t i = 0; i < numInstructions; ++i)
	{

	}
}

void printGraph(uint128_struct *instructions, uint32_t numInstructions)
{
	static std::vector<uint128_struct*> blocs;

	if (std::find(blocs.begin(), blocs.end(), instructions) != blocs.end())
		return;

	blocs.push_back(instructions);

	FILE * f = fopen(std::string("/home/ablanleu/Documents/stage/xdot/cgra"+std::to_string(blocs.size())+".dot").c_str(), "w");
	Log::fprintf(0, f, "digraph cgra {");
	for (uint32_t i = 0; i < numInstructions; ++i)
	{
		uint32_t opCode = ((instructions[i].word96>>19) & 0x7f);
		uint32_t typeCode = ((instructions[i].word96>>28) & 0x3);
		uint32_t isImm = ((instructions[i].word96>>18) & 0x1);
		uint16_t src1 = ((instructions[i].word96>>0) & 0x1ff);
		uint16_t src2 = ((instructions[i].word64>>23) & 0x1ff);
		uint16_t dst  = ((instructions[i].word64>>14) & 0x1ff);

		Log::fprintf(0, f, "i%d [label=%s];", i, opcodeNames[opCode]);

		if (typeCode == 0)
		{
			if (opCode == VEX_STD)
				if (dst < 256)
					Log::fprintf(0, f, "i%d -> i%d;", dst, i);
				else
					Log::fprintf(0, f, "r%d -> i%d;", dst-256, i);

			if (src2 < 256)
				Log::fprintf(0, f, "i%d -> i%d;", src2, i);
			else
				Log::fprintf(0, f, "r%d -> i%d;", src2-256, i);

			if (!isImm)
			{
				if (src1 < 256)
					Log::fprintf(0, f, "i%d -> i%d;", src1, i);
				else
					Log::fprintf(0, f, "r%d -> i%d;", src1-256, i);
			}
		}
	}
	Log::fprintf(0, f, "}");

	fclose(f);
}
