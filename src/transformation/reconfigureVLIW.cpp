/*
 * reconfigureVLIW.cpp
 *
 *  Created on: 9 févr. 2017
 *      Author: simon
 */


#include <isa/vexISA.h>
#include <transformation/reconfigureVLIW.h>

unsigned int schedulerConfigurations[16] = {0x00005e00,0x0000546c,0x00005ec4,0x006c54c4,0x00005ecc,0x040c5ec4,0,0,
											0x00005e64,0x44605e04,0x00005e6c,0x40605ec4,0x006c5ec4,0x446c5ec4,0,0};
char validConfigurations[12] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12 ,13};
float powerConsumptions[16] = {5.04, 7.12, 8.96, 11.03, 10.85, 12.87, 0, 0,
		9.11, 9.24, 9.11, 11.13, 13.02, 15.04};


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

	return issueWidth;

}

unsigned int getConfigurationForScheduler(char configuration){
	return schedulerConfigurations[configuration % 16];
}

unsigned int getReconfigurationInstruction(char configuration){
	unsigned int schedulerConf = getConfigurationForScheduler(configuration);

	char mux6 = (((schedulerConf>>8) & 0xf) & 4) == 0;
	char mux7 = (((schedulerConf>>4) & 0xf) & 2) != 0;
	char mux8 = (((schedulerConf>>0) & 0xf) & 8) != 0;

	char activations[8];
	activations[0] = ((schedulerConf>>(4*3)) & 0xf) != 0;
	activations[1] = ((schedulerConf>>(4*2)) & 0xf) != 0 && !mux6;
	activations[2] = ((schedulerConf>>(4*1)) & 0xf) != 0 && !mux7;
	activations[3] = ((schedulerConf>>(4*0)) & 0xf) != 0 && !mux8;
	activations[4] = ((schedulerConf>>(4*7)) & 0xf) != 0;
	activations[5] = ((schedulerConf>>(4*6)) & 0xf) != 0 || mux6;
	activations[6] = ((schedulerConf>>(4*5)) & 0xf) != 0 || mux7;
	activations[7] = ((schedulerConf>>(4*4)) & 0xf) != 0 || mux8;

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

	return assembleIInstruction_sw(VEX_RECONFFS, immediateValue, configuration);
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
	return powerConsumptions[configuration];
}

void setVLIWConfiguration(VexSimulator *simulator, char configuration){
	unsigned int schedulerConf = getConfigurationForScheduler(configuration);

	simulator->muxValues[0] = (((schedulerConf>>8) & 0xf) & 2) == 0;
	simulator->muxValues[1] = (((schedulerConf>>4) & 0xf) & 2) != 0;
	simulator->muxValues[2] = (((schedulerConf>>0) & 0xf) & 8) != 0;

	simulator->unitActivation[0] = ((schedulerConf>>(4*3)) & 0xf) != 0;
	simulator->unitActivation[1] = ((schedulerConf>>(4*2)) & 0xf) != 0 && !simulator->muxValues[0];
	simulator->unitActivation[2] = ((schedulerConf>>(4*1)) & 0xf) != 0 && !simulator->muxValues[1];
	simulator->unitActivation[3] = ((schedulerConf>>(4*0)) & 0xf) != 0 && !simulator->muxValues[2];
	simulator->unitActivation[4] = ((schedulerConf>>(4*7)) & 0xf) != 0;
	simulator->unitActivation[5] = ((schedulerConf>>(4*6)) & 0xf) != 0 || simulator->muxValues[0];
	simulator->unitActivation[6] = ((schedulerConf>>(4*5)) & 0xf) != 0 || simulator->muxValues[1];
	simulator->unitActivation[7] = ((schedulerConf>>(4*4)) & 0xf) != 0 || simulator->muxValues[2];

	simulator->issueWidth = getIssueWidth(configuration);
	char regFileControlBit = configuration>>4;

	simulator->currentConfig = configuration;

}

void changeConfiguration(IRProcedure *procedure){


	for (int oneValidConfiguration = 0; oneValidConfiguration < 12; oneValidConfiguration++){
		char oneConfiguration = validConfigurations[oneValidConfiguration];
		if (procedure->configurationScores[oneConfiguration] == 0){
			procedure->previousConfiguration = procedure->configuration;
			procedure->configuration = oneConfiguration;
			return;
		}
	}

	procedure->state = 2;

}

int computeScore(IRProcedure *procedure){
	float result = 0;
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];
		result += block->vliwEndAddress - block->vliwStartAddress;
	}
	if (getIssueWidth(procedure->configuration) > 4)
		result = result/2;

	result = 100000 / result;

	return (int) result;
}

int suggestConfiguration(IRProcedure *originalProc, IRProcedure *newlyScheduledProc){

	char currentConf = newlyScheduledProc->configuration;

	int nbInstr = getNbInstr(originalProc);
	int nbMult = getNbInstr(originalProc, 3);
	int nbMem = getNbInstr(originalProc, 1);


	int size = 0;
	for (int oneBlock = 0; oneBlock<newlyScheduledProc->nbBlock; oneBlock++){
		IRBlock *block = newlyScheduledProc->blocks[oneBlock];
		size += block->vliwEndAddress - block->vliwStartAddress;
	}
	if (getIssueWidth(newlyScheduledProc->configuration) > 4)
		size = size / 2;

	int ressourceMult = getNbMult(newlyScheduledProc->configuration);
	int ressourceMem = getNbMult(newlyScheduledProc->configuration);
	int ressourceInstr = getIssueWidth(newlyScheduledProc->configuration);

	float scoreMult = 1.0 * nbMult / (size * ressourceMult);
	float scoreMem = 1.0 * nbMem / (size * ressourceMem);
	float scoreSimple = 1.0 * nbInstr / (size * ressourceInstr);

	//fprintf(stderr, "Scores for suggestion are %f %f %f\n", scoreMult, scoreMem, scoreSimple);

	char confLowerPerf = -1, confHigherPerf = -1;
	if (scoreMult < scoreMem && scoreMult < scoreSimple){
		//score mult is the minimal
		if (nbMult > 1)
			confLowerPerf = currentConf - 2;
		else if (scoreMem < scoreSimple){
			if (nbMem > 1)
				confLowerPerf = currentConf - 8;
		}
		else{
			if (currentConf & 0x1)
				confLowerPerf = currentConf & 0x1e;
		}
	}
	else if (scoreMem < scoreMult && scoreMem < scoreSimple){
		//score mem is the minimal
		if (nbMem > 1)
			confLowerPerf = currentConf - 8;
		else if (scoreMult < scoreSimple){
			if (nbMult > 1)
				confLowerPerf = currentConf - 2;
		}
		else{
			if (currentConf & 0x1)
				confLowerPerf = currentConf & 0x1e;
		}
	}
	else {
		//Score simple is the minimal
		if (currentConf & 0x1)
			confLowerPerf = currentConf & 0x1e;
	}

	//fprintf(stderr, "Conf %x goes to %x\n", currentConf, confLowerPerf);
}


