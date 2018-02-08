#ifndef CGRAISA_H
#define CGRAISA_H

#define CGRA_NOP 0x0

// Arithmetics
#define CGRA_ADD 0x1
#define CGRA_ADDi 0x9
#define CGRA_SUB 0x2
#define CGRA_SUBi 0xA

#define CGRA_MUL 0x3
#define CGRA_DIV 0x4

#define CGRA_LDi 0xB
// End Arithmetics

// Routing instruction
#define CGRA_RT  0xF

// Masks and Offsets for instruction parts
#define IMM13 0xFFF80
#define IMM16 0xFFFF0
#define OP1   0x00070
#define OP2   0x00380

#define OP1_OFF 4
#define OP2_OFF 7
#define IMM13_OFF 7
#define IMM16_OFF 4

#endif // CGRAISA_H
