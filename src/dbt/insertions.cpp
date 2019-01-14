/*
 * insertions.cpp
 *
 *  Created on: 11 janv. 2017
 *      Author: Simon Rokicki
 */

#include <isa/vexISA.h>
#include <types.h>
#include <lib/endianness.h>
#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>
#include <transformation/irScheduler.h>
#include <transformation/reconfigureVLIW.h>


	/* Version 1.1 : TODO
	 * Will add support for multiple code areas. The idea is to start with an allocation table. Considering the address of the
	 * destination, we will be able to find the start point of an array containing insertion on the corresponding section of
	 * code. This section will be dimensioned with 1024 instructions.
	 * This allocation table will also contain the translated offset of the instruction which will be a starting value to
	 * compute the new destination.
	 *
	 * The new organization is based on the following struct :
	 *
	 *  struct insertionCodeArea {
	 *  	int numberInstr;
	 *  	int baseAddressInVLIW;
	 *  	int insertions[254];
 *  	}
	 *
	 * This struct size is 1024 bytes.
	 *
	 *
	 * How the list of insertions is organized:
	 *
	 *   | Address    |   Value stored
	 *   --------------------------------------
	 *   | 0x7000000  | struct insertionCodeArea[0]
	 *   |    ...     |      ...
	 *   | 0x7000400  | struct insertionCodeArea[1]
	 *   |    ...     |      ...
	 *
	 * If systems asks for address n, the value n/(16*1024) is used to find in which sub-array we are. Then we use bits 10
	 * to 13 to find, in the current sub-array, the code area we are concerned by.
	 * The complexity is now borned by the number of insertion in a given code area (we supposed here that it will never be
	 * more than 256 because code area will concern 1024 source instructions.
	 */


#define MAX_INSERTION_PER_SECTION 2048
#define SHIFT_FOR_INSERTION_SECTION 13 //Should be equal to log2(MAX_INSERTION_PER_SECTION) + 2



int insertionsArray[200*2048];
int unresolvedJumpsArray[200*2048];
int unresolvedJumpsTypeArray[200*2048];
int unresolvedJumpsSourceArray[200*2048];




int loadWordFromInsertionMemory(int offset){

	return insertionsArray[offset];
}

void storeWordFromInsertionMemory(int offset, int word){
	//Note offset is given in terms of bytes
	insertionsArray[offset] = word;
}


void initializeInsertionsMemory(int sizeSourceCode){
	int nbBlock = 1+( sizeSourceCode>>12);
	for (int oneBlock = 0; oneBlock<nbBlock; oneBlock++){
		int offset = oneBlock<<(SHIFT_FOR_INSERTION_SECTION-2);
		storeWordFromInsertionMemory(offset, -1);
	}
}

void addInsertions(unsigned int blockStartAddressInSources, unsigned int blockStartAddressInVLIW, unsigned int* insertionsToInsert, unsigned int numberInsertions){
	//This procedure insert a list of insertions in the global list.


	int section = blockStartAddressInSources >> 10;
	int offset = section << (SHIFT_FOR_INSERTION_SECTION-2); // globalSection * size = globalSection * 16 * (4+4) = globalSection * 0x80
	//Currently offset point to the struct corresponding to the code section.
	storeWordFromInsertionMemory(offset, numberInsertions);
	storeWordFromInsertionMemory(offset + 1, blockStartAddressInVLIW);

	//We copy the insertions
	for (int oneInsertion = 0; oneInsertion<numberInsertions; oneInsertion++){
		storeWordFromInsertionMemory(offset+oneInsertion+2, insertionsToInsert[1+oneInsertion]-blockStartAddressInVLIW);
	}

	//We fill the rest with -1
	for (int oneInsertion = numberInsertions; oneInsertion<MAX_INSERTION_PER_SECTION-2; oneInsertion++){
		storeWordFromInsertionMemory(offset+oneInsertion+2, 0x7fffffff);
	}
}


unsigned int solveUnresolvedJump(DBTPlateform *platform, unsigned int initialDestination){

	/* This procedure will receive an initial destination (eg. the destination in source binaries) and will
	 * compute and return the address of the new destination.
	 *
	 * There is a special case when jump cannot be solved because it is not in a translated area of the code.
	 * This will be determined by the tool by seeing the number of instruction at -1.
	 * In such a case, the procedure will return the value -1 which has to be understood as "not solvable yet".
	 */

//	fprintf(stderr, "While solving unrsolved jump, destination is 0x%x (%d)\n", initialDestination, initialDestination);

	int destination = 0;

	int section = initialDestination >> 10;
	int offset = section << (SHIFT_FOR_INSERTION_SECTION-2); // globalSection * size = globalSection * 16 * (4+4) = globalSection * 0x80

	//Currently offset point to the struct corresponding to the code section.
	int nbInsertion = loadWordFromInsertionMemory(offset);

	if (nbInsertion == -1)
		return -1;

	int size = MAX_INSERTION_PER_SECTION;
	int VLIWBase = loadWordFromInsertionMemory(offset + 1);
	unsigned int init = (initialDestination % 1024);
	int start = 0;

//	fprintf(stderr, "Section has %d insertions, base address is %d\n", nbInsertion, VLIWBase);

	while (size != 1){
//		fprintf(stderr, "\t Dichotomie: start = %d, size=%d, value = %d\n", start, size, loadWordFromInsertionMemory(offset + 2 + start + size/2));
		size = size / 2;

		int value = loadWordFromInsertionMemory(offset + 2 + start + size);
		if (platform->vliwInitialIssueWidth>4)
			value = value / 2;

		if (value <= init + size + start)
			start += size;
	}

	int value = loadWordFromInsertionMemory(offset + 2 + start + 0);
	if (platform->vliwInitialIssueWidth>4)
		value = value / 2;

	if (value <= init + 0 + start)
		start++;

	unsigned int result = VLIWBase + start +  (initialDestination % 1024);
	if (platform->vliwInitialIssueWidth>4)
		result = VLIWBase + (start +  (initialDestination % 1024))*2;

	return result;
}

/********************************************************************************************
 *  Function insertCodeForInsertions
 ********************************************************************************************
 *  This function will insert in VLIW binaries the code that is necessary to compute the destination of a register jump.
 *  Arguments are the following:
 *  	- platform is the DBT platform used in the framework
 *  	- start is the address in VLIW binaries where to start writing the binaries
 *  	- startAddress is the address of the first instruction in the source binaries
 *
 *  Function returns the sum of the start value and the number of instructions added (ie. the place where the rest of binaries may be written)
 ********************************************************************************************/

unsigned int insertCodeForInsertions(DBTPlateform *platform, int start, unsigned int startAddress){

	/* This procedure will solve the same problem than the previous one but it aims at being done by the VLIW processor.
	 * In here we will manually build the IR corresponding to the code computing the new destination.
	 * This IR will then be scheduled according to the VLIW initial issueWidth and configuration.
	 *
	 * The search algorithm correspond to the search algorithm describe in function solveUnresolvedJump but written in IR format.
	 * The first block initialize every thing and set the init value correctly.
	 * The loop will look in the middle of the possible insertion limit and see if we are greater or smaller. Depending on this,
	 * it will increase the begining or the end of the search path.
	 * Once the correct number of dependencies is found, the correct destination is computed and we jump there.
	 *
	 * Because of the use of the IR, this code will work on any VLIW configuration
	 */

	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;

	char offset_start=4, init_start=5, value=6, size=7, tmp1=8, tmp2=9, test1=10, test2=11;

	char increment = (platform->vliwInitialIssueWidth>4) ? 2:1;




	/*********************************************************************************************************
	 * Generation of the hashmap block
	 ********************************************************************************************************/

	char nbInstr = 0;

	write128(platform->bytecode, nbInstr*16, assembleIBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_MOVI, 256+37, 0x8, 0)); //0
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SLLi, 0, 28, 256+37, 0)); //1
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_ANDi, 256+33, 0x70, 256+34, 0)); //2
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SUB, 2, 1,  256+34, 0)); //3
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 3, -8, false, 0,  256+35, 0)); //4
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 3, -16, false, 0, 256+36, 0)); //5
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 3, -8, false, 0, 256+33, 0)); //6
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_CONTROL, 0, VEX_BRF, 4, increment*3, 256+33, 0)); //7
	nbInstr++;


	unsigned int hashBytecode[8*4];
	for (int oneBytecodeInstr = 0; oneBytecodeInstr < nbInstr; oneBytecodeInstr++){
		hashBytecode[4*oneBytecodeInstr + 0] = readInt(platform->bytecode, 16*oneBytecodeInstr + 0);
		hashBytecode[4*oneBytecodeInstr + 1] = readInt(platform->bytecode, 16*oneBytecodeInstr + 4);
		hashBytecode[4*oneBytecodeInstr + 2] = readInt(platform->bytecode, 16*oneBytecodeInstr + 8);
		hashBytecode[4*oneBytecodeInstr + 3] = readInt(platform->bytecode, 16*oneBytecodeInstr + 12);
	}

	addDataDep(hashBytecode, 0, 1);
	addDataDep(hashBytecode, 1, 3);
	addDataDep(hashBytecode, 2, 3);
	addDataDep(hashBytecode, 3, 4);
	addDataDep(hashBytecode, 3, 5);
	addDataDep(hashBytecode, 3, 6);
	addDataDep(hashBytecode, 4, 7);

	addControlDep(hashBytecode, 4,6);
	addControlDep(hashBytecode, 5,7);
	addControlDep(hashBytecode, 6,7);


	//We move instructions into bytecode memory
	for (int oneBytecodeInstr = 0; oneBytecodeInstr<nbInstr; oneBytecodeInstr++){
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, hashBytecode[4*oneBytecodeInstr + 0]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, hashBytecode[4*oneBytecodeInstr + 1]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, hashBytecode[4*oneBytecodeInstr + 2]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, hashBytecode[4*oneBytecodeInstr + 3]);
	}

	int binaSize = irScheduler(platform, 1, nbInstr, start, 32, platform->vliwInitialConfiguration);
	start += binaSize;


	/*********************************************************************************************************
	 * Generation of the hashmap block 2
	 ********************************************************************************************************/

	nbInstr = 0;

	write128(platform->bytecode, nbInstr*16, assembleIBytecodeInstruction(STAGE_CODE_CONTROL, 0, VEX_GOTOR, 256+36, 0, 0)); //0
	nbInstr++;

	unsigned int hashBytecode2[6*4];
	for (int oneBytecodeInstr = 0; oneBytecodeInstr < nbInstr; oneBytecodeInstr++){
		hashBytecode2[4*oneBytecodeInstr + 0] = readInt(platform->bytecode, 16*oneBytecodeInstr + 0);
		hashBytecode2[4*oneBytecodeInstr + 1] = readInt(platform->bytecode, 16*oneBytecodeInstr + 4);
		hashBytecode2[4*oneBytecodeInstr + 2] = readInt(platform->bytecode, 16*oneBytecodeInstr + 8);
		hashBytecode2[4*oneBytecodeInstr + 3] = readInt(platform->bytecode, 16*oneBytecodeInstr + 12);
	}


	//We move instructions into bytecode memory
	for (int oneBytecodeInstr = 0; oneBytecodeInstr<nbInstr; oneBytecodeInstr++){
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, hashBytecode2[4*oneBytecodeInstr + 0]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, hashBytecode2[4*oneBytecodeInstr + 1]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, hashBytecode2[4*oneBytecodeInstr + 2]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, hashBytecode2[4*oneBytecodeInstr + 3]);
	}

	binaSize = irScheduler(platform, 1, nbInstr, start, 32, platform->vliwInitialConfiguration);
	start += binaSize;


	/*********************************************************************************************************
	 * Generation of the first block, before the loop body
	 ********************************************************************************************************/


	nbInstr = 0;

	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+2, -8,  false, 0, 256+offset_start, 0)); //0
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+2, -16,  false, 0, 256+init_start, 0)); //1
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+2, -24,  false, 0, 256+value, 0)); //2
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+2, -32,  false, 0, 256+size, 0)); //3
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+2, -40,  false, 0, 256+tmp1, 0)); //4
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+2, -48,  false, 0, 256+tmp2, 0)); //5
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+2, -56, false, 0, 256+test1, 0)); //6
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+2, -64, false, 0, 256+test2, 0)); //7
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleIBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_MOVI, 256+offset_start, startAddress, 0)); //8
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SUB,  8, 256+33, 256+33, 0)); //9
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SRAi, 9, 12, 256+offset_start, 0)); //10
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SLLi, 10, SHIFT_FOR_INSERTION_SECTION, 256+offset_start, 0)); //11
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleIBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_MOVI, 256+size, 0x8, 0)); //12
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SLLi, 12, 28, 256+size, 0)); //13
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_ADD, 13, 11, 256+offset_start, 0)); //14
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_ANDi, 9, 4095, 256+init_start, 0)); //15
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDW, 14, 4,  false, 0, 256+33, 0)); //16
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleIBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_MOVI, 256+size, MAX_INSERTION_PER_SECTION/2, 0)); //17
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_ADD, 256+0, 14, 256+value, 0)); //18
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SH2ADD, 18, 17, 256+value, 0)); //19
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SRAi, 15, 2, 256+init_start, 0)); //20
	nbInstr++;


	unsigned int startBytecode[32*4];
	for (int oneBytecodeInstr = 0; oneBytecodeInstr < nbInstr; oneBytecodeInstr++){
		startBytecode[4*oneBytecodeInstr + 0] = readInt(platform->bytecode, 16*oneBytecodeInstr + 0);
		startBytecode[4*oneBytecodeInstr + 1] = readInt(platform->bytecode, 16*oneBytecodeInstr + 4);
		startBytecode[4*oneBytecodeInstr + 2] = readInt(platform->bytecode, 16*oneBytecodeInstr + 8);
		startBytecode[4*oneBytecodeInstr + 3] = readInt(platform->bytecode, 16*oneBytecodeInstr + 12);
	}
	addControlDep(startBytecode, 0,8);
	addDataDep(startBytecode, 8, 9);
	addDataDep(startBytecode, 9,10);
	addDataDep(startBytecode, 10,11);
	addControlDep(startBytecode, 1,15);
	addDataDep(startBytecode, 9,15);

	addDataDep(startBytecode, 14, 16);
	addControlDep(startBytecode, 9,16);

	addControlDep(startBytecode, 3,17);
	addControlDep(startBytecode, 2,18);
	addDataDep(startBytecode, 14, 18);
	addDataDep(startBytecode, 18, 19);
	addDataDep(startBytecode, 17, 19);

	addDataDep(startBytecode, 12, 13);
	addDataDep(startBytecode, 13, 14);
	addDataDep(startBytecode, 11, 14);
	addControlDep(startBytecode, 14,17);
	addDataDep(startBytecode, 15, 20);
	addControlDep(startBytecode, 3, 12);



	//We move instructions into bytecode memory
	for (int oneBytecodeInstr = 0; oneBytecodeInstr<nbInstr; oneBytecodeInstr++){
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, startBytecode[4*oneBytecodeInstr + 0]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, startBytecode[4*oneBytecodeInstr + 1]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, startBytecode[4*oneBytecodeInstr + 2]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, startBytecode[4*oneBytecodeInstr + 3]);
	}

	binaSize = irScheduler(platform, 1, nbInstr, start, 32, platform->vliwInitialConfiguration);
	start += binaSize;

	start += increment;
	/*********************************************************************************************************
	 * Generation of the loop body
	 ********************************************************************************************************/

	nbInstr = 0;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDW, 256+value, 8,  false, 0, 256+value, 0)); //0
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SRAi, 0, increment == 2 ? 1 : 0, 256+value, 0)); //1
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SH2ADD, 256+offset_start, 256+size, 256+tmp1, 0)); //2
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_ADD, 256+init_start, 256+size, 256+tmp2, 0)); //3
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_CMPLE, 3, 1, 256+test1, 0)); //4
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_CMPNE, 256+size, 256+0, 256+test2, 0)); //5
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SRAi, 256+size, 1, 256+size, 0)); //6
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SETc, 4, 2, 256+offset_start, 0)); //7
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SETc, 4, 3, 256+init_start, 0)); //8
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SH2ADD, 7, 6, 256+value, 0)); //9
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleIBytecodeInstruction(STAGE_CODE_CONTROL, 0, VEX_BR, 5, 0, 0)); //11
	nbInstr++;

	unsigned int loopBytecode[32*4];
	for (int oneBytecodeInstr = 0; oneBytecodeInstr < nbInstr; oneBytecodeInstr++){
		loopBytecode[4*oneBytecodeInstr + 0] = readInt(platform->bytecode, 16*oneBytecodeInstr + 0);
		loopBytecode[4*oneBytecodeInstr + 1] = readInt(platform->bytecode, 16*oneBytecodeInstr + 4);
		loopBytecode[4*oneBytecodeInstr + 2] = readInt(platform->bytecode, 16*oneBytecodeInstr + 8);
		loopBytecode[4*oneBytecodeInstr + 3] = readInt(platform->bytecode, 16*oneBytecodeInstr + 12);
	}

	addDataDep(loopBytecode, 0, 1);
	addDataDep(loopBytecode, 1, 4);
	addDataDep(loopBytecode, 2, 7);
	addDataDep(loopBytecode, 3, 8);
	addDataDep(loopBytecode, 3, 4);
	addDataDep(loopBytecode, 4, 7);
	addDataDep(loopBytecode, 4, 8);
	addDataDep(loopBytecode, 5, 10);
	addDataDep(loopBytecode, 6, 9);
	addDataDep(loopBytecode, 7, 9);
	addControlDep(loopBytecode, 2,6);
	addControlDep(loopBytecode, 3,6);
	addControlDep(loopBytecode, 5,6);
	addControlDep(loopBytecode, 2,7);
	addControlDep(loopBytecode, 3,8);
	addControlDep(loopBytecode, 4,8);
	addDataDep(loopBytecode, 9, 10);
	addControlDep(loopBytecode, 1,9);
	addControlDep(loopBytecode, 4,9);
	addControlDep(loopBytecode, 8,10);

	//We move instructions into bytecode memory
	for (int oneBytecodeInstr = 0; oneBytecodeInstr<nbInstr; oneBytecodeInstr++){
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, loopBytecode[4*oneBytecodeInstr + 0]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, loopBytecode[4*oneBytecodeInstr + 1]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, loopBytecode[4*oneBytecodeInstr + 2]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, loopBytecode[4*oneBytecodeInstr + 3]);
	}

	binaSize = irScheduler(platform, 1, nbInstr, start, 32, platform->vliwInitialConfiguration);
	start += binaSize ;

	writeInt(platform->vliwBinaries, (start-2*increment)*16, assembleRiInstruction_sw(VEX_BRF, test2, 0, (-(binaSize-2*increment))));


	/*********************************************************************************************************
	 * Generation of the last block, after the loop body
	 ********************************************************************************************************/
	nbInstr = 0;

	if (platform->vliwInitialIssueWidth>4)
		write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SH1ADD, 256+33, 256+init_start, 256+33, 0)); //0
	else
		write128(platform->bytecode, nbInstr*16, assembleRBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_ADD, 256+33, 256+init_start, 256+33, 0)); //0

	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_SLLi, 0, 2, 256+33, 0)); //1
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 256+2, -8,  false, 0, 256+offset_start, 0)); //2
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 256+2, -16,  false, 0, 256+init_start, 0)); //3
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 256+2, -24,  false, 0, 256+value, 0)); //4
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 256+2, -32,  false, 0, 256+size, 0)); //5
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 256+2, -40,  false, 0, 256+tmp1, 0)); //6
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 256+2, -48,  false, 0, 256+tmp2, 0)); //7
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 256+2, -56,  false, 0, 256+test1, 0)); //8
	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_LDD, 256+2, -64,  false, 0, 256+test2, 0)); //9

	nbInstr++;
	write128(platform->bytecode, nbInstr*16, assembleRiBytecodeInstruction(STAGE_CODE_ARITH, 0, VEX_ADDi, 256+33, 4*increment, 256+33, 0)); //10
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleMemoryBytecodeInstruction(STAGE_CODE_MEMORY, 0, VEX_STD, 256+34, -16,  false, 0, 1, 0)); //11
	nbInstr++;

	write128(platform->bytecode, nbInstr*16, assembleIBytecodeInstruction(STAGE_CODE_CONTROL, 0, VEX_GOTOR, 1, 0, 0)); //12
	nbInstr++;

	unsigned int endBytecode[32*4];
	for (int oneBytecodeInstr = 0; oneBytecodeInstr < nbInstr; oneBytecodeInstr++){
		endBytecode[4*oneBytecodeInstr + 0] = readInt(platform->bytecode, 16*oneBytecodeInstr + 0);
		endBytecode[4*oneBytecodeInstr + 1] = readInt(platform->bytecode, 16*oneBytecodeInstr + 4);
		endBytecode[4*oneBytecodeInstr + 2] = readInt(platform->bytecode, 16*oneBytecodeInstr + 8);
		endBytecode[4*oneBytecodeInstr + 3] = readInt(platform->bytecode, 16*oneBytecodeInstr + 12);
	}

	addDataDep(endBytecode, 0, 1);
	addDataDep(endBytecode, 1, 10);
	addDataDep(endBytecode, 10, 12);

	addControlDep(endBytecode, 2,5);
	addControlDep(endBytecode, 3,6);
	addControlDep(endBytecode, 4,7);
	addControlDep(endBytecode, 5,8);
	addControlDep(endBytecode, 6,12);
	addControlDep(endBytecode, 7,12);
	addControlDep(endBytecode, 8,12);
	addControlDep(endBytecode, 9,12);
	addControlDep(endBytecode, 11,12);



	//We move instructions into bytecode memory
	for (int oneBytecodeInstr = 0; oneBytecodeInstr<nbInstr; oneBytecodeInstr++){
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, endBytecode[4*oneBytecodeInstr + 0]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, endBytecode[4*oneBytecodeInstr + 1]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, endBytecode[4*oneBytecodeInstr + 2]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, endBytecode[4*oneBytecodeInstr + 3]);
	}

	binaSize = irScheduler(platform, 1, nbInstr, start, 32, platform->vliwInitialConfiguration);
	start += binaSize;

//	writeInt(platform->vliwBinaries, (start-2*increment)*16, assembleIInstruction(VEX_GOTOR, 0, 33));

	//This is only for debug


	return start;


//	char offset = (platform->vliwInitialIssueWidth>4) ? 2:1;
//	//		| init = startAddress| stw r4 -4(sp)		|					| r33 = r33 >> 2
//	int cycle = start;
//	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_MOVI, startAddress & 0x7ffff, 4));
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 4, 2, -8));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	//		| r33 = r33 - init	 | stw r5 -8(sp)		| 				|
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, assembleRInstruction(VEX_SUB, 33, 33, 4));
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 5, 2, -16));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	//		| 				 | 					| r33 = r33 & -4096	| init = r33 & 0xfff
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, assembleRiInstruction(VEX_ANDi, 33, 33, -4096));
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_ANDi, 4, 33, 0xfff));
//
//	//		| init -= startAddr	 | stw r6 -12(sp)		|					| offset = 7
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0 /*assembleRiInstruction(VEX_SUBi, 4, 4, startAddress & 0xfff)*/);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 6, 2, -24));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleIInstruction(VEX_MOVI, 7, 5));
//
//	//		|init = init>>2		 | stw r7 -16(sp)		|					| offset = offset<<24
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, assembleRiInstruction(VEX_SRLi, 4, 4, 2));
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 7, 2, -32));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_SLLi, 5, 5, 24));
//
//	//		| offset offset + r33<<1| stw r8 -20(sp)		|					| start = MAXNB*2
//	cycle+=offset;
//	char operation = (MAX_INSERTION_PER_SECTION == 2048) ? VEX_SH3ADD : (MAX_INSERTION_PER_SECTION == 1024) ? VEX_SH2ADD : (MAX_INSERTION_PER_SECTION == 512) ? VEX_SH1ADD :  VEX_ADD;
//	writeInt(platform->vliwBinaries, cycle*16+0, assembleRInstruction(VEX_SH1ADD, 5, 33, 5));
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 8, 2, -40));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleIInstruction(VEX_MOVI,2*MAX_INSERTION_PER_SECTION, 7));
//
//	//		| v1 = offset + start  | stw r9 -24(sp)		|					| size = 256
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, assembleRInstruction(VEX_ADD, 6, 5, 7));
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 9, 2, -48));
//	writeInt(platform->vliwBinaries, cycle*16+8, assembleIInstruction(VEX_MOVI,0, 7));
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_ADDi, 8, 0, MAX_INSERTION_PER_SECTION/2));
//
//	// bcl: | 				     | r33 = ldw 8(v1)     	|	start = 0				| init = init + size
//	cycle+=offset;
//	int bcl = cycle;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 33, 6, 8));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_ADD, 4, 4, 8));
//
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	// 		| 					 |						| 					| t1 = cmple r33 init
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_CMPLE, 9, 33, 4));
//
//	// 		|					 |						| v1 = t1 * size	| init = init - size
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, assembleRInstruction(VEX_MPY, 6, 9, 8));
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_SUB, 4, 4, 8));
//
//	// 		| t1 = cmpeqi size 1 |						|					| size = size >>1
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, assembleRiInstruction(VEX_CMPNEi, 9, 8, 1));
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12,  assembleRiInstruction(VEX_SRLi, 8, 8, 1));
//
//	// 		| 				 | start += v1			| init = init + v1	| v1 = offset + size<<2
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRInstruction(VEX_ADD, 7, 7, 6));
//	writeInt(platform->vliwBinaries, cycle*16+8, assembleRInstruction(VEX_ADD, 4, 4, 6));
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_SH2ADD, 6, 8, 5));
//
//	// 		| 				 | 					 	|					|
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	// 		| br t1	 | 					| 					| v1 = (start<<2) + v1
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_BR, (bcl-cycle)<<2, 9));
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_SH2ADD, 6, 7, 6));
//
//	// 		|					 | 					 	|					|
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	//		|					 | ldw v1 4(offset)		|					|
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 6, 5, 4));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//
//	//		|					 | ldw r4 -4(sp)		| r8 = init + v1	|
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 4, 2, -8));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_ADD, 8, 4, 6));
//
//	//		|					 | ldw r5 -8(sp)		|					| r8 ++
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 5, 2, -16));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_ADDi, 8, 8, 1));
//
//	//		|					 | ldw r6 -12(sp)		|					|
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 6, 2, -24));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	//		| 					| ldw r7 -16(sp)		|					| r8 = r8<<2
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 7, 2, -32));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_SLLi, 8, 8, platform->vliwInitialIssueWidth>4 ? 3 : 2));
//
//	//		| gotor r8			 | ldw r9 -24(sp)		|					|
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 9, 2, -48));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	//		|					 | ldw r8 -20(sp)		|					|
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_GOTOR, 0, 8));
//	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 8, 2, -40));
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	cycle+=offset;
//	writeInt(platform->vliwBinaries, cycle*16+0, 0);
//	writeInt(platform->vliwBinaries, cycle*16+4, 0);
//	writeInt(platform->vliwBinaries, cycle*16+8, 0);
//	writeInt(platform->vliwBinaries, cycle*16+12, 0);
//
//	cycle+=offset;
//	return cycle;
}

int getInsertionList(int mipsStartAddress, int** result){
	//Note: the mips start address taken is already divided by 4 (address of the instruction, not the byte)

	int destination = 0;

	int section = mipsStartAddress >> 10;
	int offset = section << (SHIFT_FOR_INSERTION_SECTION-2); // globalSection * size = globalSection * 16 * (4+4) = globalSection * 0x80

	//Currently offset point to the struct corresponding to the code section.
	int nbInsertion = loadWordFromInsertionMemory(offset);

	//todo : this should use load and store functions...
	*result = &(insertionsArray[offset + 2]);


	return nbInsertion;

}

