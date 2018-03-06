#ifndef CGRAISA_H
#define CGRAISA_H

#include <isa/vexISA.h>

constexpr uint64_t CGRA_IMM_LONG_MASK  = 0b11111111111111111111;
constexpr uint64_t CGRA_IMM_SHORT_MASK = 0b00000001111111111111;
constexpr uint64_t CGRA_OPCODE_MASK    = 0b00000000000011111111;
constexpr uint64_t CGRA_REG_MASK       = 0b00000000000001111111;

constexpr int CGRA_OPCODE_BITS = 8;
constexpr int CGRA_REG_BITS = 7;
constexpr int CGRA_IMM_SHORT_BITS = 13;
constexpr int CGRA_IMM_LONG_BITS = CGRA_IMM_SHORT_BITS + CGRA_REG_BITS;
constexpr int CGRA_INSTRUCTION_BITS = CGRA_REG_BITS + CGRA_IMM_LONG_BITS + CGRA_OPCODE_BITS;

constexpr int CGRA_IMM_OFFSET = CGRA_OPCODE_BITS;
constexpr int CGRA_REG1_OFFSET = CGRA_IMM_OFFSET + CGRA_IMM_LONG_BITS;
constexpr int CGRA_REG2_OFFSET = CGRA_REG1_OFFSET - CGRA_REG_BITS;
constexpr int CGRA_REG3_OFFSET = CGRA_REG2_OFFSET - CGRA_REG_BITS;

constexpr int CGRA_IMM_SHORT_MAX = 4096;

constexpr uint8_t CGRA_CARRY = 0x80;
constexpr uint8_t CGRA_RECONF_IF0 = 0x81;

namespace cgra
{

typedef struct
{
	int32_t i;
	int32_t j;
	int32_t k;
} cgra_node;

cgra_node operator+(const cgra_node& l, const cgra_node& r);
bool operator==(const cgra_node& l, const cgra_node& r);
bool operator!=(const cgra_node& l, const cgra_node& r);

uint8_t regA(const uint64_t& instruction);
uint8_t regB(const uint64_t& instruction);
uint8_t regC(const uint64_t& instruction);
uint8_t opcode(const uint64_t& instruction);

uint16_t immShort(const uint64_t& instruction);
uint32_t immLong(const uint64_t& instruction);

uint64_t vex2cgra(uint128_struct instruction, uint8_t src1 = 0xff, uint8_t src2 = 0xff, uint16_t * read1 = nullptr, uint16_t * read2 = nullptr);
bool isEligible(uint32_t i1, uint32_t i2, uint32_t i3, uint32_t i4);

std::string toString(uint64_t instruction);
void printConfig(int verbose, const uint64_t * configuration);

uint8_t direction(const cgra_node& from, const cgra_node& to);

} // namespace cgra

#endif // CGRAISA_H
