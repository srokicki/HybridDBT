#include <stdio.h>
#include <stdlib.h>

#include <frontend.h>
#include <lib/endianness.h>
#include <lib/tools.h>
#include <vexSimulator.h>


#ifdef __LINUX_API

#include <lib/elfFile.h>

int main(int argc, char *argv[])
{
	printf("Generation of bytecode at %s \n", argv[1]);
	ElfFile elfFile((char *) argv[1]);
	unsigned char destinationBinaries[65536];
	unsigned int placeCode = 4; //As 4 instruction bundle

	//We add initialization code
	writeInt(destinationBinaries, 0, 0x747fffd8);
	writeInt(destinationBinaries, 16, 0x223);
	writeInt(destinationBinaries, 32, 0);
	writeInt(destinationBinaries, 48, 0x2f);

	unsigned char* code;
	unsigned int addressStart;
	unsigned int size;

	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);

		if (!section->getName().compare(".text")){

			code = section->getSectionCode();
			addressStart = section->address;
			size = section->size/4;

		}
	}

	/********************************************************/
	/* First part, generation of inefficient code           */
	/********************************************************/


	//We define data structures to find block/procedure boundaries. This will be used on next stage
	short *blocksBoundaries = (short*) malloc(size * sizeof(short));
	short *proceduresBoundaries = (short*) malloc(size * sizeof(short));

	for (int i = 0; i<size; i++)
		blocksBoundaries[i] = 0;

	unsigned int numberInsertions = 0;
	int insertions[512];

	//We launch the actual generation
	firstPassTranslator(code, &size, addressStart, destinationBinaries, &placeCode, numberInsertions, insertions, blocksBoundaries, proceduresBoundaries);



	//We write back the result if needed
	void* destinationBinariesFile = openWriteFile((void*) "./binaries");
	unsigned int sizeBinaries = (placeCode<<4);

	writeFile(destinationBinariesFile, 0, 4, 1, &sizeBinaries);
	writeFile(destinationBinariesFile, 4, sizeBinaries, 1, destinationBinaries);
	closeFile(destinationBinariesFile);

//	for (int i=0;i<sizeBinaries>>4;i++){
//		printf("0x%xl, 0x%xl, 0x%xl, 0x%xl,\n", ((unsigned int*) destinationBinaries)[4*i+0],
//				((unsigned int*) destinationBinaries)[4*i+1],
//				((unsigned int*) destinationBinaries)[4*i+2],
//				((unsigned int*) destinationBinaries)[4*i+3]);
//	}



	/********************************************************/
	/********************************************************/


	/********************************************************/
	/* Second part, generation of the bytecode w/o regalloc */
	/********************************************************/


	unsigned char bytecode[16000];
	unsigned int placeBytecode = 0;


//	for (unsigned int oneInstruction = 0; oneInstruction<size; oneInstruction++){
//		printf("At %x : %d    %d\n", oneInstruction*4 +addressStart , blocksBoundaries[oneInstruction], proceduresBoundaries[oneInstruction]);
//	}
	irGenerator(code, &size, addressStart,
			bytecode, &placeBytecode,
			blocksBoundaries, proceduresBoundaries, insertions);

	//We write back the result if needed
	void* destinationBytecodeFile = openWriteFile((void*) "./bytecode.bc");
	unsigned int sizeBytecode = placeBytecode;



	writeFile(destinationBytecodeFile, 0, sizeBytecode, 1, bytecode);
	closeFile(destinationBytecodeFile);


	/********************************************************/
	/* third part, generation of the bytecode w regalloc */
	/********************************************************/

//
//	unsigned char bytecodeRenamed[16000];
//	unsigned int placeRenamedBytecode = 0;
//
//
////	for (unsigned int oneInstruction = 0; oneInstruction<size; oneInstruction++){
////		printf("At %x : %d    %d\n", oneInstruction*4 +addressStart , blocksBoundaries[oneInstruction], proceduresBoundaries[oneInstruction]);
////	}
//	generateRenamedBytecode(code, &size, addressStart,
//			bytecodeRenamed, &placeRenamedBytecode,
//			blocksBoundaries, proceduresBoundaries);
//
//	//We write back the result if needed
//	void* destinationRenamedBytecodeFile = openWriteFile((void*) "./bytecodeRenamed.bc");
//	unsigned int sizeRenamedBytecode = placeRenamedBytecode;
//
//
//
//	writeFile(destinationRenamedBytecodeFile, 0, sizeRenamedBytecode, 1, bytecodeRenamed);
//	closeFile(destinationRenamedBytecodeFile);
//
//
//	/********************************************************/
	/********************************************************/

  return 0;
}
#endif

#ifndef __LINUX_API


#include <binaries_content.h>

int main(int argc, char *argv[])
{

	unsigned char destinationBinaries[65536];
	unsigned int placeCode = 2; //As 4 instruction bundle
	/********************************************************/
	/* First part, generation of inefficient code           */
	/********************************************************/
//	startPerformances(0);


	//We define data structures to find block/procedure boundaries. This will be used on next stage
	short *blocksBoundaries = (short*) malloc(size * sizeof(short));
	short *proceduresBoundaries = (short*) malloc(size * sizeof(short));

	for (int i = 0; i<size; i++){
		blocksBoundaries[i] = 0;
		proceduresBoundaries[i] =0;
	}

	unsigned int numberInsertions = 0;
	int insertions[512];

	//We launch the actual generation
	firstPassTranslator(code, &size, addressStart, destinationBinaries, &placeCode, numberInsertions, insertions, blocksBoundaries, proceduresBoundaries);

//	stopPerformances(0);
//
//	printf("Time taken for generation %d cycles\n",  getPerformances(0));


	//We write back the result if needed
	unsigned int sizeBinaries = (placeCode>>2);

//	writeFile((void*) destination, 0, 4, 1, &sizeBinaries);
//	writeFile((void*) destination, 4, placeCode, 1, destinationBinaries);


	/********************************************************/
	/********************************************************/


	/********************************************************/
	/* Second part, generation of the bytecode w/o regalloc */
	/********************************************************/

//	startPerformances(0);


	unsigned char bytecode[16000];
	unsigned int placeBytecode = 0;


//	printf("ac_int<16, false> blocksBoundaries[size] = {");
//	for (unsigned int oneInstruction = 0; oneInstruction < size; oneInstruction++){
//		printf("%d,\n", blocksBoundaries[oneInstruction]);
//	}
//	printf("};\n short proceduresBoundaries[size] = {");
//
//	for (unsigned int oneInstruction = 0; oneInstruction < size; oneInstruction++){
//		printf("%d,\n", proceduresBoundaries[oneInstruction]);
//	}
//	printf("};\n ");
	irGenerator(code, &size, addressStart,
			bytecode, &placeBytecode,
			blocksBoundaries, proceduresBoundaries);


//	stopPerformances(0);
//
//	printf("Time taken for generation %d cycles\n",  getPerformances(0));


	//We write back the result if needed
	void* destinationBytecodeFile = openWriteFile((void*) bytecode);
	unsigned int sizeBytecode = placeBytecode;


	/********************************************************/
	/* third part, generation of the bytecode w regalloc */
	/********************************************************/
	startPerformances(0);


	unsigned char bytecodeRenamed[16000];
	unsigned int placeRenamedBytecode = 0;


//	for (unsigned int oneInstruction = 0; oneInstruction<size; oneInstruction++){
//		printf("At %x : %d    %d\n", oneInstruction*4 +addressStart , blocksBoundaries[oneInstruction], proceduresBoundaries[oneInstruction]);
//	}
	generateRenamedBytecode(code, &size, addressStart,
			bytecodeRenamed, &placeRenamedBytecode,
			blocksBoundaries, proceduresBoundaries);

	//We write back the result if needed
	void* destinationRenamedBytecodeFile = openWriteFile((void*) "./bytecodeRenamed.bc");
	unsigned int sizeRenamedBytecode = placeRenamedBytecode;

	stopPerformances(0);

	printf("Time taken for generation %d cycles\n",  getPerformances(0));

	writeFile(destinationRenamedBytecodeFile, 0, sizeRenamedBytecode, 1, bytecodeRenamed);
	closeFile(destinationRenamedBytecodeFile);

  return 0;
}
#endif
