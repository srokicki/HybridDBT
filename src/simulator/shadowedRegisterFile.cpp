#define IW 8
#include <lib/ac_int.h>

struct shadowRegFileInput {
  ac_int<6, false> rs1[IW];
  ac_int<6, false> rs2[IW];
  ac_int<6, false> rd[IW];
  ac_int<64, false> rd_data[IW];
  ac_int<1, false> rollback;
  ac_int<1, false> checkpoint;
};

struct shadowRegFileOutput {
  ac_int<64, false> rs1_data[IW];
  ac_int<64, false> rs2_data[IW];
};

ac_int<64, false> registerFile[64];
ac_int<64, false> shadowRegisterFile[64];

struct shadowRegFileOutput doShadowedRegisterFile(struct shadowRegFileInput inputs)
{
  struct shadowRegFileOutput result;

  for (int oneIssue = 0; oneIssue < IW; oneIssue++) {
    result.rs1_data[oneIssue] = registerFile[inputs.rs1[oneIssue]];
    result.rs2_data[oneIssue] = registerFile[inputs.rs2[oneIssue]];
  }

  for (int oneIssue = 0; oneIssue < IW; oneIssue++) {
    registerFile[inputs.rd[oneIssue]] = inputs.rd_data[oneIssue];
  }

  for (int oneRegister = 0; oneRegister < 64; oneRegister++) {
    if (inputs.checkpoint) {
      shadowRegisterFile[oneRegister] = registerFile[oneRegister];
    } else if (inputs.rollback) {
      registerFile[oneRegister] = shadowRegisterFile[oneRegister];
    }
  }

  return result;
}
