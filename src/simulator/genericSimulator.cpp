/*
 * genericSimulator.cpp
 *
 *  Created on: 25 avr. 2017
 *      Author: simon
 */

#ifndef __NIOS

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <types.h>
#include <simulator/genericSimulator.h>


void GenericSimulator::initialize(int argc, char** argv){

	//We initialize registers
	for (int oneReg = 0; oneReg < 32; oneReg++)
		REG[oneReg] = 0;
	REG[2] = 0xf00000;

	/******************************************************
	 * Argument passing:
	 * In this part of the initialization code, we will copy argc and argv into the simulator stack memory.
	 *
	 ******************************************************/

	ac_int<64, true> currentPlaceStrings = REG[2] + 8 + 8*argc;

	this->std(REG[2], argc);
	for (int oneArg = 0; oneArg<argc; oneArg++){
		this->std(REG[2] + 8*oneArg + 8, currentPlaceStrings);


		int oneCharIndex = 0;
		char oneChar = argv[oneArg][oneCharIndex];
		while (oneChar != 0){
			this->stb(currentPlaceStrings + oneCharIndex, oneChar);
			oneCharIndex++;
			oneChar = argv[oneArg][oneCharIndex];
		}
		this->stb(currentPlaceStrings + oneCharIndex, oneChar);
		oneCharIndex++;
		currentPlaceStrings += oneCharIndex;

	}

}



void GenericSimulator::stb(ac_int<64, false> addr, ac_int<8, true> value){
	this->memory[addr] = value & 0xff;

}



void GenericSimulator::sth(ac_int<64, false> addr, ac_int<16, true> value){
	this->stb(addr+1, value.slc<8>(8));
	this->stb(addr+0, value.slc<8>(0));
}

void GenericSimulator::stw(ac_int<64, false> addr, ac_int<32, true> value){
	this->stb(addr+3, value.slc<8>(24));
	this->stb(addr+2, value.slc<8>(16));
	this->stb(addr+1, value.slc<8>(8));
	this->stb(addr+0, value.slc<8>(0));

}

void GenericSimulator::std(ac_int<64, false> addr, ac_int<64, true> value){
	this->stb(addr+7, value.slc<8>(56));
	this->stb(addr+6, value.slc<8>(48));
	this->stb(addr+5, value.slc<8>(40));
	this->stb(addr+4, value.slc<8>(32));
	this->stb(addr+3, value.slc<8>(24));
	this->stb(addr+2, value.slc<8>(16));
	this->stb(addr+1, value.slc<8>(8));
	this->stb(addr+0, value.slc<8>(0));
}


ac_int<8, true> GenericSimulator::ldb(ac_int<64, false> addr){

	ac_int<8, true> result = 0;
	if (this->memory.find(addr) != this->memory.end())
		result = this->memory[addr];
	else
		result= 0;

//	fprintf(stderr, "memread %x %x\n", addr, result);
	return result;
}


//Little endian version
ac_int<16, true> GenericSimulator::ldh(ac_int<64, false> addr){

	ac_int<16, true> result = 0;
	result.set_slc(8, this->ldb(addr+1));
	result.set_slc(0, this->ldb(addr));
	return result;
}

ac_int<32, true> GenericSimulator::ldw(ac_int<64, false> addr){

	ac_int<32, true> result = 0;
	result.set_slc(24, this->ldb(addr+3));
	result.set_slc(16, this->ldb(addr+2));
	result.set_slc(8, this->ldb(addr+1));
	result.set_slc(0, this->ldb(addr));
	return result;
}

ac_int<64, true> GenericSimulator::ldd(ac_int<64, false> addr){

	ac_int<64, true> result = 0;
	result.set_slc(56, this->ldb(addr+7));
	result.set_slc(48, this->ldb(addr+6));
	result.set_slc(40, this->ldb(addr+5));
	result.set_slc(32, this->ldb(addr+4));
	result.set_slc(24, this->ldb(addr+3));
	result.set_slc(16, this->ldb(addr+2));
	result.set_slc(8, this->ldb(addr+1));
	result.set_slc(0, this->ldb(addr));

	return result;
}





ac_int<64, false> GenericSimulator::solveSyscall(ac_int<64, false> syscallId, ac_int<64, false> arg1, ac_int<64, false> arg2, ac_int<64, false> arg3, ac_int<64, false> arg4){
	ac_int<64, false> result = 0;
	switch (syscallId){
		case SYS_exit:
			stop = 1; //Currently we break on ECALL
		break;
		case SYS_read:
			result = this->doRead(arg1, arg2, arg3);
		break;
		case SYS_write:
			result = doWrite(arg1, arg2, arg3);
		break;
		case SYS_brk:
			result = this->doSbrk(arg1);
		break;
		case SYS_open:
			result = this->doOpen(arg1, arg2, arg3);
		break;
		case SYS_openat:
			result = this->doOpenat(arg1, arg2, arg3, arg4);
		break;
		case SYS_lseek:
			result = this->doLseek(arg1, arg2, arg3);
		break;
		case SYS_close:
			result = this->doClose(arg1);
		break;
		case SYS_fstat:
			result = 0;
		break;
		case SYS_stat:
			result = this->doStat(arg1, arg2);
		break;
		case SYS_gettimeofday:
			result = doGettimeofday(arg1);
		break;
		case SYS_unlink:
			result = this->doUnlink(arg1);
		break;
		default:
			printf("Unknown syscall with code %d\n", syscallId.slc<32>(0));
			exit(-1);
		break;
		}
	return result;

}

ac_int<64, false> GenericSimulator::doRead(ac_int<64, false> file, ac_int<64, false> bufferAddr, ac_int<64, false> size){
	//printf("Doign read on file %x\n", file);

	int localSize = size.slc<32>(0);
	char* localBuffer = (char*) malloc(localSize*sizeof(char));
	ac_int<64, false> result;

	if (file == 0){
		if (nbInStreams == 1)
			result = fread(localBuffer, 1, size, inStreams[0]);
		else
			result = fread(localBuffer, 1, size, stdin);
	}
	else{
		FILE* localFile = this->fileMap[file.slc<16>(0)];
		result = fread(localBuffer, 1, size, localFile);
		if (localFile == 0)
			return -1;
	}

	for (int i=0; i<result; i++){
		this->stb(bufferAddr + i, localBuffer[i]);
	}

	return result;
}


ac_int<64, false> GenericSimulator::doWrite(ac_int<64, false> file, ac_int<64, false> bufferAddr, ac_int<64, false> size){
	int localSize = size.slc<32>(0);
	char* localBuffer = (char*) malloc(localSize*sizeof(char));
	for (int i=0; i<size; i++)
		localBuffer[i] = this->ldb(bufferAddr + i);

	if (file < 5){
		ac_int<64, false> result = 0;
		int streamNB = (int) file-nbInStreams;
		if (nbOutStreams + nbInStreams > file)
			result = fwrite(localBuffer, 1, size, outStreams[streamNB]);
		else
			result = fwrite(localBuffer, 1, size, stdout);
		return result;
	}
	else{

		FILE* localFile = this->fileMap[file.slc<16>(0)];
		if (localFile == 0)
			return -1;

		ac_int<64, false> result = fwrite(localBuffer, 1, size, localFile);
		return result;
	}
}


ac_int<64, false> GenericSimulator::doOpen(ac_int<64, false> path, ac_int<64, false> flags, ac_int<64, false> mode){
	int oneStringElement = this->ldb(path);
	int index = 0;
	while (oneStringElement != 0){
		index++;
		oneStringElement = this->ldb(path+index);
	}

	int pathSize = index+1;

	char* localPath = (char*) malloc(pathSize*sizeof(char));
	for (int i=0; i<pathSize; i++)
		localPath[i] = this->ldb(path + i);

	char* localMode;
	if (flags==0)
		localMode = "r";
	else if (flags == 577)
		localMode = "w";
	else if (flags == 1089)
		localMode = "a";
	else if (flags == O_WRONLY|O_CREAT|O_EXCL)
		localMode = "wx";
	else{
		fprintf(stderr, "Trying to open files with unknown flags... %d\n", flags);
		exit(-1);
	}

	FILE* test = fopen(localPath, localMode);
	uint64_t result = (uint64_t) test;
	ac_int<64, true> result_ac = result;

	//For some reasons, newlib only store last 16 bits of this pointer, we will then compute a hash and return that.
	//The real pointer is stored here in a hashmap

	ac_int<64, true> returnedResult = 0;
	returnedResult.set_slc(0, result_ac.slc<15>(0) ^ result_ac.slc<15>(16));
	returnedResult[15] = 0;

	this->fileMap[returnedResult.slc<16>(0)] = test;



	return returnedResult;

}

ac_int<64, false> GenericSimulator::doOpenat(ac_int<64, false> dir, ac_int<64, false> path, ac_int<64, false> flags, ac_int<64, false> mode){
	fprintf(stderr, "Syscall openat not implemented yet...\n");
	exit(-1);
}

ac_int<64, false> GenericSimulator::doClose(ac_int<64, false> file){
	if (file > 2 ){
		FILE* localFile = this->fileMap[file.slc<16>(0)];
		int result = fclose(localFile);
		return result;
	}
	else
		return 0;
}

ac_int<64, true> GenericSimulator::doLseek(ac_int<64, false> file, ac_int<64, false> ptr, ac_int<64, false> dir){
	if (file>2){
		FILE* localFile = this->fileMap[file.slc<16>(0)];
		if (localFile == 0)
			return -1;
		int result = fseek(localFile, ptr, dir);
		return result;
	}
	else
		return 0;
}

ac_int<64, false> GenericSimulator::doStat(ac_int<64, false> filename, ac_int<64, false> ptr){

	int oneStringElement = this->ldb(filename);
	int index = 0;
	while (oneStringElement != 0){
		index++;
		oneStringElement = this->ldb(filename+index);
	}

	int pathSize = index+1;

	char* localPath = (char*) malloc(pathSize*sizeof(char));
	for (int i=0; i<pathSize; i++)
		localPath[i] = this->ldb(filename + i);

	struct stat fileStat;
	int result = stat(localPath, &fileStat);

	//We copy the result in simulator memory
	for (int oneChar = 0; oneChar<sizeof(struct stat); oneChar++)
		this->stb(ptr+oneChar, ((char*)(&stat))[oneChar]);

	return result;
}

ac_int<64, false> GenericSimulator::doSbrk(ac_int<64, false> value){
	if (value == 0){
		return this->heapAddress;
	}
	else {
		this->heapAddress = value;
		return value;
	}
}

ac_int<64, false> GenericSimulator::doGettimeofday(ac_int<64, false> timeValPtr){
	timeval* oneTimeVal;
	struct timezone* oneTimeZone;
	int result = gettimeofday(oneTimeVal, oneTimeZone);

//	this->std(timeValPtr, oneTimeVal->tv_sec);
//	this->std(timeValPtr+8, oneTimeVal->tv_usec);

	return result;


}

ac_int<64, false> GenericSimulator::doUnlink(ac_int<64, false> path){
	int oneStringElement = this->ldb(path);
	int index = 0;
	while (oneStringElement != 0){
		index++;
		oneStringElement = this->ldb(path+index);
	}

	int pathSize = index+1;

	char* localPath = (char*) malloc(pathSize*sizeof(char));
	for (int i=0; i<pathSize; i++)
		localPath[i] = this->ldb(path + i);


	int result = unlink(localPath);

	return result;

}

#endif
