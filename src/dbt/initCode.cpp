#include <lib/ac_int.h>
#include <lib/endianness.h>
#include <isa/vexISA.h>

#define JUMP_RESOLVE 4

#define SP_REG 2
#define STACK_ORIENTATION -1


#ifdef __NIOS
unsigned int getInitCode(unsigned int *binaries, int start, unsigned int startAddress){
#endif

#ifdef __USE_AC
unsigned int getInitCode(ac_int<128, false> *binaries, int start, unsigned int startAddress){
#endif

	int cycle = start;
	writeInt(binaries, cycle*16+0, assembleIInstruction(VEX_CALL, 0, 0));
	writeInt(binaries, cycle*16+4, 0); //OLD : assembleIInstruction(VEX_MOVI,0xffff,29)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleIInstruction(VEX_STOP, 0, 0));
	writeInt(binaries, cycle*16+4, 0);		//STOP
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_ADDi, 4, 0, 0x700)); 	//r4 = 0x700
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_STW, 4, SP_REG, -4));		//stw r4 0(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_SLLi, 4, 4, 16));		//r4 = r4 << 16
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_STW, 5, SP_REG, -8));		//stw r5 4(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, assembleIInstruction(VEX_MOVI, (startAddress>>14), 5));	//r5 = 0xa0025 FIXME param

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_SLLi, 5, 5, 14));		//r5 = r5 << 14
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_STW, 6, SP_REG, -12));		//stw r6 8(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_ADDi, 5, 5, (startAddress & 0x3fff)));	//r5 = r5 + 0x40 -> r5 = 0xa0020040
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_STW, 7, SP_REG, -16));		//stw r7 12(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_ADDi, 4, 4, 4));		//r4 = r4 + 4 = start
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_STW, 8, SP_REG, -20));		//stw r8 16(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, assembleRInstruction(VEX_SUB, 33, 33, 5));		//r33 = r33 - r5 (=0xa0020040)

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_SRAi, 33, 33, 2));		//r33 = r33 / 4
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 5, 4, -4));		//r5 = ldw -4(r4) = size
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_SLLi, 5, 5, 2));		//r5 = r5 * 4
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_ADDi, 33, 33, 26));		//r33 = r33 + 26
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	int instrWithPlaceCode = cycle;
	writeInt(binaries, cycle*16+0, assembleRInstruction(VEX_ADD, 5, 5, 4));			//r5 = r5 + r4 = end
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	//bcl
	cycle++;
	int bcl = cycle;

	writeInt(binaries, cycle*16+0, assembleRInstruction(VEX_CMPEQ, 7, 4, 5));
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleIInstruction(VEX_BR, 8, 7));
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 8, 4, 0));
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRInstruction(VEX_CMPGT, 7, 8, 33));
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleIInstruction(VEX_BR, 4, 7));
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleIInstruction(VEX_GOTO, bcl, 0));
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_ADDi, 33, 33, 1));
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_ADDi, 4, 4, 4));
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	//finBcl
	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 4, SP_REG, -4));		//ldw r4 0(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 5, SP_REG, -8));		//ldw r5 4(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 6, SP_REG, -12));		//ldw r6 8(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, assembleIInstruction(VEX_GOTOR, 0, 33));
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 7, SP_REG, -16));		//ldw r7 12(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 8, SP_REG, -20));		//ldw r8 16(sp)
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);
	cycle++;

	writeInt(binaries, instrWithPlaceCode*16+4, assembleRiInstruction(VEX_ADDi, 33, 33, 0));
	writeInt(binaries, start*16+0, assembleIInstruction(VEX_CALL, cycle, 63));


	return cycle;

}
