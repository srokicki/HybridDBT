#include <isa/riscvISA.h>
#include <simulator/vexSimulator.h>

struct DCtoEx decodeRISCV(ac_int<32, false> oneInstruction, bool enable, bool& stall)
{
  static ac_int<8, false> decoderState = 0;
  static struct DCtoEx nextPipelineRegisters;
  static int lastLatency, lastWrittenRegister;
  struct DCtoEx result = {0};

  if (enable == 0) {
    if (decoderState == 0) {
      // Default state of the decoder: we analyze the incoming instruction

      ac_int<7, false> opcode = oneInstruction.slc<7>(0);
      ac_int<7, false> rs1    = oneInstruction.slc<5>(15);
      ac_int<7, false> rs2    = oneInstruction.slc<5>(20);
      ac_int<7, false> rs3    = oneInstruction.slc<5>(27);

      ac_int<6, false> rd             = oneInstruction.slc<5>(7);
      ac_int<7, false> funct7         = oneInstruction.slc<7>(25);
      ac_int<7, false> funct7_smaller = 0;
      funct7_smaller.set_slc(1, oneInstruction.slc<6>(26));

      ac_int<5, false> funct_atom = funct7.slc<5>(2);

      ac_int<3, false> funct3   = oneInstruction.slc<3>(12);
      ac_int<12, false> imm12_I = oneInstruction.slc<12>(20);
      ac_int<12, false> imm12_S = 0;
      imm12_S.set_slc(5, oneInstruction.slc<7>(25));
      imm12_S.set_slc(0, oneInstruction.slc<5>(7));

      ac_int<12, true> imm12_I_signed = oneInstruction.slc<12>(20);
      ac_int<12, true> imm12_S_signed = 0;
      imm12_S_signed.set_slc(0, imm12_S.slc<12>(0));

      ac_int<13, false> imm13 = 0;
      imm13[12]               = oneInstruction[31];
      imm13.set_slc(5, oneInstruction.slc<6>(25));
      imm13.set_slc(1, oneInstruction.slc<4>(8));
      imm13[11] = oneInstruction[7];

      ac_int<13, true> imm13_signed = 0;
      imm13_signed.set_slc(0, imm13);

      ac_int<32, false> imm31_12 = 0;
      imm31_12.set_slc(12, oneInstruction.slc<20>(12));

      ac_int<32, true> imm31_12_signed = 0;
      imm31_12_signed.set_slc(0, imm31_12);

      ac_int<21, false> imm21_1 = 0;
      imm21_1.set_slc(12, oneInstruction.slc<8>(12));
      imm21_1[11] = oneInstruction[20];
      imm21_1.set_slc(1, oneInstruction.slc<10>(21));
      imm21_1[20]                     = oneInstruction[31];
      ac_int<21, true> imm21_1_signed = 0;
      imm21_1_signed.set_slc(0, imm21_1);

      ac_int<6, false> shamt = oneInstruction.slc<6>(20);

      if (rs1 == 1)
        rs1 = 63;

      if (rs2 == 1)
        rs2 = 63;

      if (rd == 1)
        rd = 63;

      if (opcode == RISCV_FMADD || opcode == RISCV_FMSUB || opcode == RISCV_FNMADD || opcode == RISCV_FNMSUB ||
          (opcode == RISCV_FP && (funct7 != RISCV_FP_FCVTS && funct7 != RISCV_FP_FMVW))) {
        rs1 += 64;
      }

      if (opcode == RISCV_FSW || opcode == RISCV_FMADD || opcode == RISCV_FMSUB || opcode == RISCV_FNMADD ||
          opcode == RISCV_FNMSUB ||
          (opcode == RISCV_FP && funct7 != RISCV_FP_SQRT && funct7 != RISCV_FP_FCVTW && funct7 != RISCV_FP_FMVXFCLASS &&
           funct7 != RISCV_FP_FCVTS && funct7 != RISCV_FP_FMVW)) {
        rs2 += 64;
      }

      ac_int<1, false> isRdFloat =
          (funct7 != RISCV_FP_FCVTW) && (funct7 != RISCV_FP_FCMP) && (funct7 != RISCV_FP_FMVXFCLASS);

      //***************************************************************************************
      // We generate the correct output

      if (opcode == RISCV_OP) {
        // Instrucion io OP type: it needs two registers operand (except for rol/sll/sr)

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;

        if (funct7 == RISCV_OP_M) {
          // We are in the part dedicated to RV32M extension
          binaries = assembleRInstruction(functBindingMULT[funct3], rd, rs1, rs2);
          stage    = stageMult;

          // nextInstructionNop = 1;
          // TODO: should certainly insert a nop
          lastLatency = MULT_LATENCY;
        } else if (funct3 == RISCV_OP_SLL || funct3 == RISCV_OP_SR) {
          // Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
          // are working with...

          ac_int<7, false> vexOpcode =
              (funct3 == RISCV_OP_SLL) ? VEX_SLL : (funct7 == RISCV_OP_SR_SRA) ? VEX_SRA : VEX_SRL;

          binaries    = assembleRInstruction(vexOpcode, rd, rs1, rs2);
          lastLatency = SIMPLE_LATENCY;
        } else {
          ac_int<7, false> subOpcode = VEX_SUB;
          ac_int<7, false> vexOpcode = (funct7 == RISCV_OP_ADD_SUB) ? subOpcode : functBindingOP[funct3];
          binaries                   = assembleRInstruction(vexOpcode, rd, rs1, rs2);
          lastLatency                = SIMPLE_LATENCY;
        }

      } else if (opcode == RISCV_AUIPC) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        lastLatency             = SIMPLE_LATENCY;

        ac_int<32, false> value = codeSectionStart + (indexInSourceBinaries << 2) + imm31_12_signed;

        if (previousIsBoundary) {

          binaries              = assembleIInstruction(VEX_MOVI, value.slc<19>(12), rd);
          nextInstruction       = assembleRiInstruction(VEX_SLLi, rd, rd, 12);
          secondNextInstruction = assembleRiInstruction(VEX_ADDi, rd, rd, value.slc<12>(0));

          // Mark the insertion
          nextInstruction_rd  = rd;
          nextInstruction_rs1 = rd;
          nextInstruction_rs2 = 0;

          secondNextInstruction_rd  = rd;
          secondNextInstruction_rs1 = rd;
          secondNextInstuction_rs2  = 0;

          nextInstruction_stage       = 0;
          enableNextInstruction       = 1;
          enableSecondNextInstruction = 1;
          secondNextInstruction_stage = 0;

        } else {
          binaries              = assembleIInstruction(VEX_MOVI, value.slc<19>(12), rd);
          nextInstruction       = assembleRiInstruction(VEX_SLLi, rd, rd, 12);
          secondNextInstruction = assembleRiInstruction(VEX_ADDi, rd, rd, value.slc<12>(0));

          // Mark the insertion
          nextInstruction_rd  = rd;
          nextInstruction_rs1 = rd;
          nextInstruction_rs2 = 0;

          secondNextInstruction_rd  = rd;
          secondNextInstruction_rs1 = rd;
          secondNextInstuction_rs2  = 0;

          nextInstruction_stage       = 0;
          enableNextInstruction       = 1;
          enableSecondNextInstruction = 1;
          secondNextInstruction_stage = 0;
        }

      } else if (opcode == RISCV_LUI) {
        // Operation Load Upper Immediate is turned into a MOVI and a SLLi instructions.
        // If the instruction is not at the start point of a block, we can add the movi at the previous
        // cycle to prevent an insertion. Otherwise, we have to do the insertion...

        // TODO RISCV immediates are on 20 bits whereas we only handle 19-bit immediates...

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        lastLatency             = SIMPLE_LATENCY;

        ac_int<1, false> keepHigher = 0;
        ac_int<1, false> keepBoth   = 0;

        if (imm31_12[31] != imm31_12[30]) {
          keepHigher = 1;
          if (imm31_12[12])
            keepBoth = 1;
        }

        ac_int<19, false> immediateValue = keepHigher ? imm31_12.slc<19>(13) : imm31_12.slc<19>(12);
        ac_int<5, false> shiftValue      = keepHigher ? 13 : 12;

        ac_int<32, false> instr1 = assembleIInstruction(VEX_MOVI, immediateValue, rd);
        ac_int<32, false> instr2 = assembleRiInstruction(VEX_SLLi, rd, rd, shiftValue);
        ac_int<32, false> instr3 = assembleRiInstruction(VEX_ADDi, rd, rd, 0x1000);

        if (previousIsBoundary) {
          binaries        = instr1;
          nextInstruction = instr2;

          // Mark the insertion
          nextInstruction_rd  = rd;
          nextInstruction_rs1 = rd;
          nextInstruction_rs2 = 0;

          nextInstruction_stage = 0;
          enableNextInstruction = 1;

          if (keepBoth) {
            secondNextInstruction_rd  = rd;
            secondNextInstruction_rs1 = rd;
            secondNextInstuction_rs2  = 0;

            secondNextInstruction       = instr3;
            enableSecondNextInstruction = 1;
            secondNextInstruction_stage = 0;
          }

        } else {
          binaries        = instr1;
          nextInstruction = instr2;

          // Mark the insertion
          nextInstruction_rd  = rd;
          nextInstruction_rs1 = rd;
          nextInstruction_rs2 = 0;

          nextInstruction_stage = 0;
          enableNextInstruction = 1;

          if (keepBoth) {
            secondNextInstruction_rd  = rd;
            secondNextInstruction_rs1 = rd;
            secondNextInstuction_rs2  = 0;

            secondNextInstruction       = instr3;
            enableSecondNextInstruction = 1;
            secondNextInstruction_stage = 0;
          }
        }

      } else if (opcode == RISCV_LD) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        lastLatency             = MEMORY_LATENCY;

        // Memory access operations.
        binaries = assembleMemoryInstruction(functBindingLD[funct3], rd, rs1, imm12_I_signed, 0, 0);
        stage    = stageMem;

      } else if (opcode == RISCV_ST) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        lastLatency             = 0;

        binaries = assembleMemoryInstruction(functBindingST[funct3], rs2, rs1, imm12_S_signed, 0, 0);
        stage    = stageMem;
      } else if (opcode == RISCV_JAL) {

        // If rsd is equal to zero, then we are in a simple J instruction
        ac_int<1, false> isSimpleJ = (rd == 0);
        binaries                   = assembleIInstruction(isSimpleJ ? VEX_GOTO : VEX_CALL, 0, rd);

        // We fill information on block boundaries
        setBoundaries1 = 1;
        boundary1      = indexInSourceBinaries + (imm21_1_signed >> 2);
        setBoundaries2 = 1;
        boundary2      = indexInSourceBinaries + 1; // Only plus one because in riscv next instr is not executed

        unresolved_jump_src  = indexInDestinationBinaries;
        unresolved_jump_type = binaries;
        setUnresolvedJump    = 1;

        // In order to deal with the fact that RISCV do not execute the isntruction following a branch,
        // we have to insert nop
        nextInstructionNop = 1;

      } else if (opcode == RISCV_JALR) {

        // JALR instruction is a bit particular: it load a value from a register and add it an immediate value.
        // Once done, it jump and store return address in rd.

        setBoundaries1 = 1;
        boundary1      = indexInSourceBinaries + 1; // Only plus one because in riscv next instr is not executed

        if (rs1 == 63 && rd == 0) {
          // We are in a simple return
          binaries = assembleIInstruction(VEX_GOTOR, imm12_I_signed, 63);

        } else {
          // FIXME should be able to add two instr at the same cycle... This would remove an insertion
          binaries = assembleRiInstruction(VEX_ADDi, 33, rs1, imm12_I_signed);

          nextInstruction       = assembleIInstruction((rd == 63) ? VEX_CALL : VEX_GOTO, 4 * incrementInDest, rd);
          enableNextInstruction = 1;
          nextInstruction_rd    = 0;
          nextInstruction_rs1   = rd;
          nextInstruction_rs2   = 0;
        }

        // In order to deal with the fact that RISCV do not execute the isntruction following a branch,
        // we have to insert nop
        nextInstructionNop = 1;

      } else if (opcode == RISCV_BR) {

        // While handling BR instruction we distinguish the case where we compare to zero cause VEX has
        // some special instruction to handle this...

        // First we fill information on block boundaries

        //				if (rs2 == 0 && (funct3 == RISCV_BR_BEQ || funct3 == RISCV_BR_BNE)){
        // This is a comparison to zero
        setBoundaries1 = 1;
        boundary1      = ((imm13_signed >> 2) + indexInSourceBinaries);

        setBoundaries2 = 1;
        boundary2      = indexInSourceBinaries + 1; // Only plus one because in riscv next instr is not executed

        binaries = assembleRiInstruction(functBindingBR[funct3], rs2, rs1, 0);

        unresolved_jump_src  = indexInDestinationBinaries;
        unresolved_jump_type = binaries;
        setUnresolvedJump    = 1;

        // In order to deal with the fact that RISCV do not execute the isntruction following a branch,
        // we have to insert nop
        nextInstructionNop = 1;

      } else if (opcode == RISCV_OPI) { // For all other instructions

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        lastLatency             = SIMPLE_LATENCY;

        ac_int<13, true> extendedImm = imm12_I_signed;
        if (funct3 == RISCV_OPI_SLTIU)
          extendedImm = imm12_I;

        if (funct3 == RISCV_OPI_SLLI || funct3 == RISCV_OPI_SRI) {

          // Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
          // are working with...

          ac_int<7, false> vexOpcode =
              (funct3 == RISCV_OPI_SLLI) ? VEX_SLLi : (funct7_smaller == RISCV_OPI_SRI_SRAI) ? VEX_SRAi : VEX_SRLi;

          binaries = assembleRiInstruction(vexOpcode, rd, rs1, shamt);

        } else {
          binaries = assembleRiInstruction(functBindingOPI[funct3], rd, rs1, extendedImm);
        }
      } else if (opcode == RISCV_OPW) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        lastLatency             = SIMPLE_LATENCY;

        if (funct7 == RISCV_OP_M) {
          // We are in the part dedicated to RV64M extension
          binaries = assembleRInstruction(functBindingMULTW[funct3], rd, rs1, rs2);
          stage    = stageMult;

          nextInstructionNop = 1;

        } else if (funct3 == RISCV_OPW_SLLW || funct3 == RISCV_OPW_SRW) {
          // Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
          // are working with...

          ac_int<7, false> vexOpcode =
              (funct3 == RISCV_OPW_SLLW) ? VEX_SLLW : (funct7 == RISCV_OPW_SRW_SRAW) ? VEX_SRAW : VEX_SRLW;

          binaries = assembleRInstruction(vexOpcode, rd, rs1, rs2);

        } else {

          ac_int<7, false> vexOpcode = (funct7 == RISCV_OPW_ADDSUBW_SUBW) ? VEX_SUBW : VEX_ADDW;
          binaries                   = assembleRInstruction(vexOpcode, rd, rs1, rs2);
        }
      } else if (opcode == RISCV_OPIW) { // For 32-bits instructions (labelled with a W)

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        lastLatency             = SIMPLE_LATENCY;

        ac_int<13, true> extendedImm = imm12_I_signed;
        if (funct3 == RISCV_OPI_SLTIU)
          extendedImm = imm12_I;

        if (funct3 == RISCV_OPIW_SLLIW || funct3 == RISCV_OPIW_SRW) {

          // Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
          // are working with...

          ac_int<7, false> vexOpcode =
              (funct3 == RISCV_OPIW_SLLIW) ? VEX_SLLWi : (funct7 == RISCV_OPIW_SRW_SRAIW) ? VEX_SRAWi : VEX_SRLWi;

          binaries = assembleRiInstruction(vexOpcode, rd, rs1, shamt.slc<5>(0));

        } else {
          binaries = assembleRiInstruction(VEX_ADDWi, rd, rs1, extendedImm);
        }
      } else if (opcode == RISCV_SYSTEM) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastLatency             = SIMPLE_LATENCY;

        if (funct3 == RISCV_SYSTEM_ENV) {
          setBoundaries1      = 1;
          boundary1           = indexInSourceBinaries; // Only plus one because in riscv next instr is not executed
          lastWrittenRegister = 10;

          binaries = assembleIInstruction(VEX_SYSTEM, VEX_SYSTEM_ECALL, 0);
        } else if (funct3 == RISCV_SYSTEM_CSRRS) {
          binaries            = assembleIInstruction(VEX_SYSTEM, VEX_SYSTEM_CSRRS, rd);
          lastWrittenRegister = rd;

        } else {

#ifndef __CATAPULT
          binaries = assembleIInstruction(VEX_NOP, 0, 0);

#endif
        }
      } else if (opcode == RISCV_FLW) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = 64 + rd; // the written reg is a float reg
        lastLatency             = MEMORY_LATENCY;

        binaries = assembleMemoryInstruction(VEX_FLW, rd, rs1, imm12_I_signed, 0, 0);

        stage = stageMem;
      } else if (opcode == RISCV_FSW) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        lastLatency             = 0;

        binaries = assembleMemoryInstruction(VEX_FSW, rs2, rs1, imm12_S_signed, 0, 0);

        stage = stageMem;

      } else if (opcode == RISCV_FMADD || opcode == RISCV_FNMADD || opcode == RISCV_FMSUB || opcode == RISCV_FNMSUB) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = 64 + rd;
        lastLatency             = MULT_LATENCY;

        if (opcode == RISCV_FMADD)
          binaries = assembleRRInstruction(VEX_FMADD, rd, rs1, rs2, rs3);
        else if (opcode == RISCV_FNMADD)
          binaries = assembleRRInstruction(VEX_FNMADD, rd, rs1, rs2, rs3);
        else if (opcode == RISCV_FMSUB)
          binaries = assembleRRInstruction(VEX_FMSUB, rd, rs1, rs2, rs3);
        else
          binaries = assembleRRInstruction(VEX_FNMSUB, rd, rs1, rs2, rs3);

        stage = stageMult;
      } else if (opcode == RISCV_FP) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = rd;
        if (isRdFloat)
          lastWrittenRegister += 64;
        lastLatency = MULT_LATENCY;

        ac_int<5, false> funct = 0;
        switch (funct7) {
          case RISCV_FP_ADD:
            funct = VEX_FP_FADD;
            break;
          case RISCV_FP_SUB:
            funct = VEX_FP_FSUB;
            break;
          case RISCV_FP_MUL:
            funct = VEX_FP_FMUL;
            break;
          case RISCV_FP_DIV:
            funct = VEX_FP_FDIV;
            break;
          case RISCV_FP_SQRT:
            funct = VEX_FP_FSQRT;
            rs2   = rs1;
            break;
          case RISCV_FP_FSGN:
            if (funct3 == RISCV_FP_FSGN_J)
              funct = VEX_FP_FSGNJ;
            else if (funct3 == RISCV_FP_FSGN_JN)
              funct = VEX_FP_FSGNJN;
            else // JX
              funct = VEX_FP_FSGNJNX;
            break;
          case RISCV_FP_MINMAX:
            if (funct3 == RISCV_FP_MINMAX_MIN)
              funct = VEX_FP_FMIN;
            else
              funct = VEX_FP_FMAX;
            break;
          case RISCV_FP_FCVTW:
            if (rs2 == RISCV_FP_FCVTW_W)
              funct = VEX_FP_FCVTWS;
            else
              funct = VEX_FP_FCVTWUS;

            rs2 = rs1;
            break;
          case RISCV_FP_FMVXFCLASS:
            if (funct3 == RISCV_FP_FMVXFCLASS_FMVX) {
              funct = VEX_FP_FMVXW;
            } else
              funct = VEX_FP_FCLASS;

            rs2 = rs1;
            break;
          case RISCV_FP_FCMP:
            if (funct3 == RISCV_FP_FCMP_FEQ)
              funct = VEX_FP_FEQ;
            else if (funct3 == RISCV_FP_FCMP_FLT)
              funct = VEX_FP_FLT;
            else
              funct = VEX_FP_FLE;
            break;
          case RISCV_FP_FCVTS:
            if (rs2 == RISCV_FP_FCVTS_W) {
              funct = VEX_FP_FCVTSW;
            } else {
              funct = VEX_FP_FCVTSWU;
            }
            rs2 = rs1;
            break;
          case RISCV_FP_FMVW:
            funct = VEX_FP_FMVWX;
            rs2   = rs1;
            break;
        }

        binaries = assembleFPInstruction(VEX_FP, funct, rd, rs1, rs2);
        stage    = stageMult;

      } else if (opcode == RISCV_ATOM) {

        binaries = assembleRiInstruction(funct3 == 0x2 ? VEX_LDW : VEX_LDD, rd, rs1, 0);

        enableNextInstruction       = 1;
        enableSecondNextInstruction = 1;

        nextInstruction_rd  = rd;
        nextInstruction_rs1 = rd;
        nextInstruction_rs2 = rs2;

        switch (funct_atom) {
          case RISCV_ATOM_LR:
            enableNextInstruction       = 0;
            enableSecondNextInstruction = 0;
            break;
          case RISCV_ATOM_SC:
            nextInstruction = assembleRiInstruction(VEX_ADDi, rd, 0, 0);
            break;
          case RISCV_ATOM_SWAP:
            nextInstruction             = assembleRiInstruction(VEX_STD, rd, rs1, 0);
            nextInstruction_rs1         = rs1;
            enableSecondNextInstruction = 0;
            break;
          case RISCV_ATOM_ADD:
            nextInstruction = assembleRInstruction(VEX_ADD, rd, rd, rs2);
            break;
          case RISCV_ATOM_XOR:
            nextInstruction = assembleRInstruction(VEX_XOR, rd, rd, rs2);
            break;
          case RISCV_ATOM_AND:
            nextInstruction = assembleRInstruction(VEX_AND, rd, rd, rs2);
            break;
          case RISCV_ATOM_OR:
            nextInstruction = assembleRInstruction(VEX_OR, rd, rd, rs2);
            break;
          case RISCV_ATOM_MIN:
            nextInstruction = assembleRInstruction(VEX_ADD, rd, rd, rs2); // TODO This is wrong !
            break;
          case RISCV_ATOM_MAX:
            nextInstruction = assembleRInstruction(VEX_ADD, rd, rd, rs2); // TODO This is wrong !
            break;
          case RISCV_ATOM_MINU:
            nextInstruction = assembleRInstruction(VEX_ADD, rd, rd, rs2); // TODO This is wrong !
            break;
          case RISCV_ATOM_MAXU:
            nextInstruction = assembleRInstruction(VEX_ADD, rd, rd, rs2); // TODO This is wrong !
            break;
        }

        secondNextInstruction     = assembleRiInstruction(funct3 == 0x2 ? VEX_STW : VEX_STD, rd, rs1, 0);
        secondNextInstruction_rs1 = rd;

      } else if (opcode == 0 || opcode == RISCV_FENCE) {

        previousLatency         = lastLatency;
        previousWrittenRegister = lastWrittenRegister;
        lastWrittenRegister     = 0;
        lastLatency             = 0;
        binaries                = 0;
      } else {
#ifndef __CATAPULT
        Log::logError << "In first pass translator, instr " << std::hex << oneInstruction << " is not handled yet...\n";
        exit(-1);
#endif
      }
    }
  }
}
else
{
  decoderState = 0;
  return result;
}
}
