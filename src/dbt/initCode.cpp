#include <dbt/dbtPlateform.h>
#include <lib/endianness.h>
#include <isa/vexISA.h>
#include <transformation/reconfigureVLIW.h>



unsigned int getInitCode(DBTPlateform *platform, int start, unsigned int startAddress){
	char offset = (platform->vliwInitialIssueWidth>4) ? 2:1;
	int cycle = start;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction_sw(VEX_CALL, 63, 0));
	writeInt(platform->vliwBinaries, cycle*16+4, 0); //OLD : assembleIInstruction(VEX_MOVI,0xffff,29)
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	cycle+=offset;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	cycle+=offset;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction_sw(VEX_STOP, 0, 0));
	writeInt(platform->vliwBinaries, cycle*16+4, 0);		//STOP
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	cycle+=offset;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);



	cycle+=offset;
	return cycle;

}
