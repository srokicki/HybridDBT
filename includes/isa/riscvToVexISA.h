/*
 * riscvToVexISA.h
 *
 *  Created on: 2 déc. 2016
 *      Author: simon
 */

#ifndef INCLUDES_ISA_RISCVTOVEXISA_H_
#define INCLUDES_ISA_RISCVTOVEXISA_H_

#include <isa/vexISA.h>
#include <types.h>

#ifndef __SW
#ifndef __HW

extern ac_int<7, false> functBindingOP[8];
extern ac_int<7, false> functBindingOPI[8];
extern ac_int<7, false> functBindingLD[8];
extern ac_int<7, false> functBindingST[8];
extern ac_int<7, false> functBindingBR[8];
extern ac_int<7, false> functBindingMULT[8];
extern ac_int<7, false> functBindingMULTW[8];

#endif
#endif

extern char functBindingOP_sw[8];
extern char functBindingOPI_sw[8];
extern char functBindingLD_sw[8];
extern char functBindingST_sw[8];
extern char functBindingBR_sw[8];
extern char functBindingMULT_sw[8];
extern char functBindingMULTW_sw[8];

#endif /* INCLUDES_ISA_RISCVTOVEXISA_H_ */
