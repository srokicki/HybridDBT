#include <lib/ac_int.h>
#include <lib/endianness.h>
#include <isa/vexISA.h>



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
	return cycle;

}
