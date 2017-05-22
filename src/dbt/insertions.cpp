/*
 * insertions.cpp
 *
 *  Created on: 11 janv. 2017
 *      Author: Simon Rokicki
 */

#include <types.h>
#include <lib/endianness.h>
#include <isa/vexISA.h>
#include <dbt/dbtPlateform.h>

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



int insertionsArray[170*2048];
int unresolvedJumpsArray[170*2048];
int unresolvedJumpsTypeArray[170*2048];
int unresolvedJumpsSourceArray[170*2048];




int loadWordFromInsertionMemory(int offset){

	return insertionsArray[offset];
}

void storeWordFromInsertionMemory(int offset, int word){
	//Note offset is given in terms of bytes
	insertionsArray[offset] = word;
}


void initializeInsertionsMemory(int sizeSourceCode){
	printf("%x\n", sizeSourceCode);
	int nbBlock = 1+( sizeSourceCode>>12);
	for (int oneBlock = 0; oneBlock<nbBlock; oneBlock++){
		int offset = oneBlock<<(SHIFT_FOR_INSERTION_SECTION-2);
		storeWordFromInsertionMemory(offset, -1);
	}
}

void addInsertions(uint32 blockStartAddressInSources, uint32 blockStartAddressInVLIW, uint32* insertionsToInsert, uint32 numberInsertions){
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


unsigned int solveUnresolvedJump(unsigned int initialDestination){

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

		if (value <= init + size + start)
			start += size;
	}

	if (loadWordFromInsertionMemory(offset + 2 + start + 0) <= init + 0 + start)
		start++;

	return VLIWBase + start +  (initialDestination % 1024);
}


unsigned int insertCodeForInsertions(DBTPlateform *platform, int start, unsigned int startAddress){

	/* This procedure will solve the same problem than the previous one but it aims at being done by the VLIW processor.
	 * In here we define directly in binary the code to run.
	 *
	 *
	 * Here is the code we will execute:
	 *
	 *		| init = startAddress| stw r4 -4(sp)		|					| 							r4 = init
	 *		| r33 = r33 - init	 | stw r5 -8(sp)		| 					|
	 *							 |						| r33 = r33 & -4096	| init = r33 & 0x1fff		r5 = offset
	 *		| init -= startAddr	 | stw r6 -12(sp)		|					| offset = 7				r6 = v1
	 *		| init = init>>2	 | stw r7 -16(sp)		|					| offset = offset<<24	 	r7 = start
	 *		| offset offset + r33<<1| stw r8 -20(sp)	|					| start = MAXNB*4					r8 = size
	 *		| v1 = offset + start  | stw r9 -24(sp)		|	start=0				| size = 256				r9 = t1
	 *
	 *
	 * bcl: | 				     | r33 = ldw 8(v1)     	|					| init = init + size
	 * 		|					 |                      |                   |
	 * 		| 					 |						| 					| t1 = cmpte r33 init
	 * 		|					 |						| v1 = t1 * size	| init = init - size
	 * 		| t1 = cmpeqi size 1 | 						|				  	|size = size >>1
	 * 		|					 | start += v1			| init = init + v1	| v1 = offset + size<<2
	 * 		|  br t1 bcl		 |                      |                   |
	 * 		| v1 = start<<2 + v1 | 					 	|					|
	 *
	 *		|					 | ldw v1 4(offset)		|					|
	 *		|					 |						|					|
	 *		|					 | ldw r4 -4(sp)		| 					|r8 = init + v1
	 *		|					 | ldw r5 -8(sp)		|					|r8++
	 *		|					 | ldw r6 -12(sp)		|					|r8 = r8<<2
	 *		|					 | ldw r7 -16(sp)		|					|
	 *		| gotor r8			 | ldw r8 -20(sp)		|					|
	 *		|					 |						|					|


	 * Same with a different pipeline latency (1 2 2 1)
	 *		| init = startAddress| stw r4 -4(sp)		|					| 							r4 = init
	 *		| r33 = r33 - init	 | stw r5 -8(sp)		| 					|
	 *							 |						| r33 = r33 & -4096	| init = r33 & 0x1fff		r5 = offset
	 *		| init -= startAddr	 | stw r6 -12(sp)		|					| offset = 7				r6 = v1
	 *		| init = init>>2	 | stw r7 -16(sp)		|					| offset = offset<<24	 	r7 = start
	 *		| offset offset + r33<<1| stw r8 -20(sp)	|					| start = 0					r8 = size
	 *		| v1 = offset + 1024  | stw r9 -24(sp)		|					| size = 256				r9 = t1
	 *
	 *
	 * bcl: | 				     | r33 = ldw 8(v1)     	|					|
	 *      | 				     |                   	|					| init = init + size
	 * 		|r33=offset+size<<1  |						|					|
	 * 		| 					 |init = init - size    | 					| t1 = cmpte r33 init
	 * 		|r33+= start << 2    |						| 					|
	 * 		| 					 | size = size >>1		| v1 = t1 * size	|t1 = cmpeqi size 1
	 * 		| 					 | start += v1			| init = init + v1	|
	 *      | 				     |                   	|					|
	 * 		| br t1 bcl			 | 					 	|					|v1=r33+v1<<2
	 *      | 				     |                   	|					|
	 *
	 *		|					 | ldw v1 4(offset)		|					|r8 = init + 1
	 *		|					 | ldw r4 -4(sp)		| 					|
	 *		|					 | ldw r5 -8(sp)		|					|
	 *		|					 | ldw r6 -12(sp)		|					|r8 = r8 + v1
	 *		| 					 | ldw r7 -16(sp)		|					|
	 *		|					 | ldw r8 -20(sp)		|					|r8 = r8<<2
	 *      | 				     |                   	|					|
	 *      |gotor r8		     |                   	|					|
	 *      | 				     |                   	|					|
	 *
	 *	//TODO handle case where code is not translated
	 */

	//		| init = startAddress| stw r4 -4(sp)		|					| r33 = r33 >> 2
	int cycle = start;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_MOVI, startAddress & 0x7ffff, 4));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 4, 2, -8));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| r33 = r33 - init	 | stw r5 -8(sp)		| 				|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRInstruction(VEX_SUB, 33, 33, 4));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 5, 2, -16));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| 				 | 					| r33 = r33 & -4096	| init = r33 & 0xfff
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, assembleRiInstruction(VEX_ANDi, 33, 33, -4096));
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_ANDi, 4, 33, 0xfff));

	//		| init -= startAddr	 | stw r6 -12(sp)		|					| offset = 7
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRiInstruction(VEX_SUBi, 4, 4, startAddress & 0xfff));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 6, 2, -24));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, assembleIInstruction(VEX_MOVI, 7, 5));

	//		|init = init>>2		 | stw r7 -16(sp)		|					| offset = offset<<24
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRiInstruction(VEX_SRLi, 4, 4, 2));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 7, 2, -32));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_SLLi, 5, 5, 24));

	//		| offset offset + r33<<1| stw r8 -20(sp)		|					| start = MAXNB*2
	cycle++;
	char operation = (MAX_INSERTION_PER_SECTION == 2048) ? VEX_SH3ADD : (MAX_INSERTION_PER_SECTION == 1024) ? VEX_SH2ADD : (MAX_INSERTION_PER_SECTION == 512) ? VEX_SH1ADD :  VEX_ADD;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRInstruction(VEX_SH1ADD, 5, 33, 5));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 8, 2, -40));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, assembleIInstruction(VEX_MOVI,2*MAX_INSERTION_PER_SECTION, 7));

	//		| v1 = offset + start  | stw r9 -24(sp)		|					| size = 256
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRInstruction(VEX_ADD, 6, 5, 7));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STD, 9, 2, -48));
	writeInt(platform->vliwBinaries, cycle*16+8, assembleIInstruction(VEX_MOVI,0, 7));
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_ADDi, 8, 0, MAX_INSERTION_PER_SECTION/2));

	// bcl: | 				     | r33 = ldw 8(v1)     	|	start = 0				| init = init + size
	cycle++;
	int bcl = cycle;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 33, 6, 8));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_ADD, 4, 4, 8));

	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	// 		| 					 |						| 					| t1 = cmple r33 init
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_CMPLE, 9, 33, 4));

	// 		|					 |						| v1 = t1 * size	| init = init - size
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, assembleRInstruction(VEX_MPYLO, 6, 9, 8));
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_SUB, 4, 4, 8));

	// 		| t1 = cmpeqi size 1 |						|					| size = size >>1
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRiInstruction(VEX_CMPNEi, 9, 8, 1));
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12,  assembleRiInstruction(VEX_SRLi, 8, 8, 1));

	// 		| 				 | start += v1			| init = init + v1	| v1 = offset + size<<2
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRInstruction(VEX_ADD, 7, 7, 6));
	writeInt(platform->vliwBinaries, cycle*16+8, assembleRInstruction(VEX_ADD, 4, 4, 6));
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_SH2ADD, 6, 8, 5));

	// 		| br t1	 | 					| 					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_BR, (bcl-cycle)<<2, 9));
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	// 		| v1 = (start<<2) + v1	 | 					 	|					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRInstruction(VEX_SH2ADD, 6, 7, 6));
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		|					 | ldw v1 4(offset)		|					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 6, 5, 4));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);


	//		|					 | ldw r4 -4(sp)		| r8 = init + v1	|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 4, 2, -8));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRInstruction(VEX_ADD, 8, 4, 6));

	//		|					 | ldw r5 -8(sp)		|					| r8 ++
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 5, 2, -16));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_ADDi, 8, 8, 1));

	//		|					 | ldw r6 -12(sp)		|					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 6, 2, -24));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| 					| ldw r7 -16(sp)		|					| r8 = r8<<2
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 7, 2, -32));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, assembleRiInstruction(VEX_SLLi, 8, 8, 2));

	//		| gotor r8			 | ldw r9 -24(sp)		|					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 9, 2, -48));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		|					 | ldw r8 -20(sp)		|					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_GOTOR, 0, 8));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDD, 8, 2, -40));
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

