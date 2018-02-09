#include <transformation/cgraScheduler.h>
#include <isa/vexISA.h>

#include <lib/log.h>

CgraScheduler::CgraScheduler()
{

}

void CgraScheduler::schedule(CgraSimulator& cgra, uint32_t * instructions, uint32_t numInstructions)
{
  // build dependency graph
  uint32_t * first_deps = new uint32_t[numInstructions];
  uint32_t * second_deps = new uint32_t[numInstructions];

  for (unsigned int i = numInstructions-1; i > 0; --i)
  {
    Log::printf(0, "fuck\n");
    uint32_t instr = instructions[i];

    uint8_t RA = (instr >> 26) & 0x3f;
    uint8_t RB = (instr >> 20) & 0x3f;
    uint8_t RC = (instr >> 14) & 0x3f;
    uint32_t IMM19 = (instr >> 7) & 0x7ffff;
    uint16_t IMM13 = (instr >> 7) & 0x1fff;
    uint16_t IMM13_signed = (IMM13 > 4095) ? IMM13 - 8192 : IMM13;
    uint8_t funct = (instr >> 7) & 0x1f;
    uint8_t OP = (instr & 0x7f);
    uint8_t BEXT = (instr >> 8) & 0x7;
    uint16_t IMM9 = (instr >> 11) & 0x1ff;

    bool binary;
    uint8_t src1, src2;
    uint32_t dep1 = -1, dep2 = -1;

    bool isImm = ((OP >> 4) & 0x7) == 1 || ((OP >> 4) & 0x7) == 6 || ((OP >> 4) & 0x7) == 7;

    if (isImm)
    {
      binary = false;
      src1 = RA;
    }
    else
    {
      binary = true;
      src1 = RA;
      src2 = RB;
    }

    for (int j = i-1; j >= 0; --j)
    {
      Log::printf(0, "fuck2 %d\n", j);
      uint32_t instr2 = instructions[j];
      uint8_t OP2 = (instr2 & 0x7f);
      uint8_t RB2 = (instr2 >> 20) & 0x3f;
      uint8_t RC2 = (instr2 >> 14) & 0x3f;

      bool isImm = ((OP2 >> 4) & 0x7) == 1 || ((OP2 >> 4) & 0x7) == 6 || ((OP2 >> 4) & 0x7) == 7;
      uint8_t RD = isImm ? RB2 : RC2;

      if (dep1 == -1 && RD == src1)
      {
        dep1 = j;
      }

      if (binary && dep2 != -1 && RD == src2)
      {
        dep2 = j;
      }
    }

    first_deps[i] = dep1;
    second_deps[i] = dep2;
  }

  for (unsigned int i = 0; i < numInstructions; ++i)
  {
    Log::printf(0, "%d, %d\n", first_deps[i], second_deps[i]);
  }

  // place & route
}
