/*
 * riscvISA.cpp
 *
 *  Created on: 7 déc. 2016
 *      Author: Simon Rokicki
 */

#include <iomanip>
#include <isa/riscvISA.h>
#include <sstream>
#include <string.h>

#ifndef __NIOS

#include <strings.h>

const char* riscvNamesOP[8]   = {"ADD", "SLL", "CMPLT", "CMPLTU", "XOR", "", "OR", "AND"};
const char* riscvNamesOPI[8]  = {"ADDi", "SLLi", "SLTi", "CMPLTUi", "XORi", "SRLi", "ORi", "ANDi"};
const char* riscvNamesOPW[8]  = {"ADDW", "SLLW", "", "", "", "SRW", "", ""};
const char* riscvNamesOPIW[8] = {"ADDWi", "SLLWi", "", "", "", "SRi", "", ""};
const char* riscvNamesLD[8]   = {"LDB", "LDH", "LDW", "LDD", "LDBU", "LDHU", "LDWU"};
const char* riscvNamesST[8]   = {"STB", "STH", "STW", "STD"};
const char* riscvNamesBR[8]   = {"BEQ", "BNE", "", "", "BLT", "BGE", "BLTU", "BGEU"};
const char* riscvNamesMUL[8]  = {"MPYLO", "MPYHI", "MPYHI", "MPYHI", "DIVHI", "DIVHI", "DIVLO", "DIVLO"};

std::string printDecodedInstrRISCV(unsigned int oneInstruction)
{
  char opcode = oneInstruction & 0x7f;
  char rs1    = ((oneInstruction >> 15) & 0x1f);
  char rs2    = ((oneInstruction >> 20) & 0x1f);
  char rd     = ((oneInstruction >> 7) & 0x1f);
  char funct7 = ((oneInstruction >> 25) & 0x7f);

  char funct3            = ((oneInstruction >> 12) & 0x7);
  unsigned short imm12_I = ((oneInstruction >> 20) & 0xfff);
  unsigned short imm12_S = ((oneInstruction >> 20) & 0xfe0) + ((oneInstruction >> 7) & 0x1f);

  short imm12_I_signed = (imm12_I >= 2048) ? imm12_I - 4096 : imm12_I;
  short imm12_S_signed = (imm12_S >= 2048) ? imm12_S - 4096 : imm12_S;

  short imm13 = ((oneInstruction >> 19) & 0x1000) + ((oneInstruction >> 20) & 0x7e0) + ((oneInstruction >> 7) & 0x1e) +
                ((oneInstruction << 4) & 0x800);
  short imm13_signed = (imm13 >= 4096) ? imm13 - 8192 : imm13;

  unsigned int imm31_12 = oneInstruction & 0xfffff000;

  char shamt = ((oneInstruction >> 20) & 0x3f);

  if (opcode == RISCV_OPIW) // If we are on opiw, shamt only have 5bits
    shamt = rs2;

  // In case of immediate shift instr, as shamt needs one more bit the lower bit of funct7 has to be set to 0
  if (opcode == RISCV_OPI && (funct3 == RISCV_OPI_SLLI || funct3 == RISCV_OPI_SRI))
    funct7 = funct7 & 0x3f;

  std::stringstream stream;

  switch (opcode) {
    case RISCV_LUI:
      stream << "LUI r" + std::to_string(rd) + " " + std::to_string(imm31_12);
      break;
    case RISCV_AUIPC:
      stream << "AUIPC r" + std::to_string(rd) + " " + std::to_string(imm31_12);
      break;
    case RISCV_JAL:
      if (rd == 0)
        stream << "J " + std::to_string(imm31_12);
      else
        stream << "JAL " + std::to_string(imm31_12);
      break;
    case RISCV_JALR:
      if (rd == 0)
        stream << "JR " + std::to_string(imm31_12);
      else
        stream << "JALR " + std::to_string(imm31_12);
      break;
    case RISCV_BR:
      stream << riscvNamesBR[funct3];
      stream << " r" + std::to_string(rs1) + " r" + std::to_string(rs2) + " " + std::to_string(imm13_signed);
      break;
    case RISCV_LD:
      stream << riscvNamesLD[funct3];
      stream << " r" + std::to_string(rd) + " = " + std::to_string(imm12_I_signed) + " (" + std::to_string(rs1) + ")";
      break;
    case RISCV_ST:
      stream << riscvNamesST[funct3];
      stream << " r" + std::to_string(rs2) + " = " + std::to_string(imm12_S_signed) + " (" + std::to_string(rs1) + ")";
      break;
    case RISCV_OPI:
      if (funct3 == RISCV_OPI_SRI)
        if (funct7 == RISCV_OPI_SRI_SRLI)
          stream << "SRLi r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", " + std::to_string(shamt);
        else // SRAI
          stream << "SRAi r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", " + std::to_string(shamt);
      else if (funct3 == RISCV_OPI_SLLI) {
        stream << riscvNamesOPI[funct3];
        stream << " r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", " + std::to_string(shamt);
      } else {
        stream << riscvNamesOPI[funct3];
        stream << " r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", " + std::to_string(imm12_I_signed);
      }
      break;
    case RISCV_OP:
      if (funct7 == 1) {
        stream << riscvNamesMUL[funct3];
        stream << " r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
      } else {
        if (funct3 == RISCV_OP_ADD)
          if (funct7 == RISCV_OP_ADD_ADD)
            stream << "ADD r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
          else
            stream << "SUB r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
        else if (funct3 == RISCV_OP_SR)
          if (funct7 == RISCV_OP_SR_SRL)
            stream << "SRL r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
          else // SRA
            stream << "SRA r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
        else {
          stream << riscvNamesOP[funct3];
          stream << " r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
        }
      }
      break;
    case RISCV_OPIW:
      if (funct3 == RISCV_OPIW_SRW)
        if (funct7 == RISCV_OPIW_SRW_SRLIW)
          stream << "SRLWi r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", " + std::to_string(rs2);
        else // SRAI
          stream << "SRAWi r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", " + std::to_string(rs2);
      else if (funct3 == RISCV_OPIW_SLLIW) {
        stream << riscvNamesOPI[funct3];
        stream << " r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", " + std::to_string(rs2);
      } else {
        stream << riscvNamesOPIW[funct3];
        stream << " r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", " + std::to_string(imm12_I_signed);
      }
      break;
    case RISCV_OPW:
      if (funct7 == 1) {
        stream << riscvNamesMUL[funct3];
        stream << "W r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
      } else {
        if (funct3 == RISCV_OP_ADD)
          if (funct7 == RISCV_OPW_ADDSUBW_ADDW)
            stream << "ADDW r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
          else
            stream << "SUBW r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
        else if (funct3 == RISCV_OPW_SRW)
          if (funct7 == RISCV_OPW_SRW_SRLW)
            stream << "SRLW r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
          else // SRAW
            stream << "SRAW r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
        else {
          stream << riscvNamesOPW[funct3];
          stream << " r" + std::to_string(rd) + " = r" + std::to_string(rs1) + ", r" + std::to_string(rs2);
        }
      }
      break;
    case RISCV_SYSTEM:
      stream << "SYSTEM";
      break;
    default:
      stream << "??? ";
      break;
  }

  std::string result(stream.str());
  for (int addedSpace = result.size(); addedSpace < 20; addedSpace++)
    result.append(" ");

  return result;
}
#endif
