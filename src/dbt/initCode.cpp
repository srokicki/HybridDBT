#include <dbt/dbtPlateform.h>
#include <lib/endianness.h>
#include <isa/vexISA.h>




unsigned int getInitCode(DBTPlateform *platform, int start, unsigned int startAddress){

	int cycle = start;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_CALL, 0, 0));
	writeInt(platform->vliwBinaries, cycle*16+4, 0); //OLD : assembleIInstruction(VEX_MOVI,0xffff,29)
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_STOP, 0, 0));
	writeInt(platform->vliwBinaries, cycle*16+4, 0);		//STOP
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	cycle++;
	return cycle;

}
