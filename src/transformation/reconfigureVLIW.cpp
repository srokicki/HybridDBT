/*
 * reconfigureVLIW.cpp
 *
 *  Created on: 9 févr. 2017
 *      Author: simon
 */


#include <isa/vexISA.h>
#include <transformation/reconfigureVLIW.h>

unsigned int schedulerConfigurations[16] = {0x00001a00,0x00001428,0x00001a84,0x04281084,0x00001a88,0x04081a84,0,0,
											0x00001a24,0x44201a04,0x00001a28,0x40201a84,0x00281a84,0x44281a84,0,0};

char getIssueWidth(char configuration){
	//This function returns the issue width of each configuration code

	//We extract variables
	char regCode = (configuration >> 4) & 0x1;
	char memCode = (configuration >> 3) & 0x1;
	char multCode = (configuration >> 1) & 0x3;
	char boostCode = configuration & 0x1;

	//Sum resources of mem and mult and round to the upper even number (this is the or)
	char issueWidth = (memCode + multCode + 2);

	if (issueWidth & 0x1)
		issueWidth++;

	//If boost code then it is the version with increased issue width
	if (boostCode)
		issueWidth += 2;

	fprintf(stdout, "conf %x, memcode %d, multcode %d, boostCode %d\n", configuration, memCode, multCode, boostCode);

	return issueWidth;

}

unsigned int getConfigurationForScheduler(char configuration){
	return schedulerConfigurations[configuration & 0xf];
}

unsigned int getReconfigurationInstruction(char configuration){
	unsigned int schedulerConf = getConfigurationForScheduler(configuration);

	char mux6 = ((schedulerConf>>8) & 0xf) == 4;
	char mux7 = ((schedulerConf>>4) & 0xf) == 2;
	char mux8 = ((schedulerConf>>0) & 0xf) == 8;

	char activations[8];
	activations[0] = ((schedulerConf>>(4*3)) & 0xf) != 0;
	activations[1] = ((schedulerConf>>(4*2)) & 0xf) != 0 && !mux6;
	activations[2] = ((schedulerConf>>(4*1)) & 0xf) != 0 && !mux7;
	activations[3] = ((schedulerConf>>(4*0)) & 0xf) != 0 && !mux8;
	activations[4] = ((schedulerConf>>(4*7)) & 0xf) != 0;
	activations[5] = ((schedulerConf>>(4*6)) & 0xf) != 0 || mux6;
	activations[6] = ((schedulerConf>>(4*5)) & 0xf) != 0 || mux7;
	activations[7] = ((schedulerConf>>(4*7)) & 0xf) != 0 || mux8;

	char issueWidth = getIssueWidth(configuration);
	char regFileControlBit = configuration>>4;

	unsigned int immediateValue = activations[0]
								+ (activations[1]<<1)
								+ (activations[2]<<2)
								+ (activations[3]<<3)
								+ (activations[4]<<4)
								+ (activations[5]<<5)
								+ (activations[6]<<6)
								+ (activations[7]<<7)

								+ (mux6<<8)
								+ (mux7<<9)
								+ (mux8<<10)

								+ (issueWidth<<11)
								+ (regFileControlBit<<15);


	return assembleIInstruction(VEX_RECONFFS, immediateValue, 0);
}

char getNbMem(char configuration){
	char memCode = (configuration >> 3) & 0x1;
	return memCode + 1;
}

char getNbMult(char configuration){
	char multCode = (configuration >> 1) & 0x3;
	return multCode + 1;
}

float getPowerConsumption(char configuration){
	//TODO
	return 0;
}

void setVLIWConfiguration(VexSimulator *simulator, char configuration){
	unsigned int schedulerConf = getConfigurationForScheduler(configuration);

	simulator->muxValues[0] = ((schedulerConf>>8) & 0xf) == 4;
	simulator->muxValues[1] = ((schedulerConf>>4) & 0xf) == 2;
	simulator->muxValues[2] = ((schedulerConf>>0) & 0xf) == 8;

	simulator->unitActivation[0] = ((schedulerConf>>(4*3)) & 0xf) != 0;
	simulator->unitActivation[1] = ((schedulerConf>>(4*2)) & 0xf) != 0 && !simulator->muxValues[0];
	simulator->unitActivation[2] = ((schedulerConf>>(4*1)) & 0xf) != 0 && !simulator->muxValues[1];
	simulator->unitActivation[3] = ((schedulerConf>>(4*0)) & 0xf) != 0 && !simulator->muxValues[2];
	simulator->unitActivation[4] = ((schedulerConf>>(4*7)) & 0xf) != 0;
	simulator->unitActivation[5] = ((schedulerConf>>(4*6)) & 0xf) != 0 || simulator->muxValues[0];
	simulator->unitActivation[6] = ((schedulerConf>>(4*5)) & 0xf) != 0 || simulator->muxValues[1];
	simulator->unitActivation[7] = ((schedulerConf>>(4*7)) & 0xf) != 0 || simulator->muxValues[2];

	simulator->issueWidth = getIssueWidth(configuration);
	char regFileControlBit = configuration>>4;

}
