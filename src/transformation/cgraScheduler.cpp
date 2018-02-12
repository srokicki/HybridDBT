#include <transformation/cgraScheduler.h>

#include <lib/log.h>

#include <cstdio>

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
  FILE * f = fopen("/home/ablanleu/Documents/stage/xdot/cgra.dot", "w");
  Log::fprintf(0, f, "digraph cgra {");
  for (uint32_t i = 0; i < numInstructions; ++i)
  {
    Log::printf(0, "%s", printBytecodeInstruction(i, instructions[i].word96
                                                  , instructions[i].word64
                                                  , instructions[i].word32
                                                  , instructions[i].word0).c_str());

    uint32_t typeCode = ((instructions[i].word96>>28) & 0x3);
    uint32_t isImm = ((instructions[i].word96>>18) & 0x1);
    uint16_t src1 = ((instructions[i].word96>>0) & 0x1ff);
    uint16_t src2 = ((instructions[i].word64>>23) & 0x1ff);

    if (typeCode == 0)
    {
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
