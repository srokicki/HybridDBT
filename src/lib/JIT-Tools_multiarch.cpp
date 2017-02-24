	/************************************
	 * Functions to handle different environments (Nios Soft / Nios Hard / Linux)
	 ************************************/
#include <stdio.h>
#include <stdlib.h>

#include <types.h>


/************************************/
	//Functions to accede bytecode and binaries

	#ifdef __LINUX_API
	void* openReadFile(void* path){
		return fopen((char*) path, "r");
	}

	void* openWriteFile(void* path){
		return fopen((char*) path, "w+");
	}

	void closeFile(void* file){
		fclose((FILE*) file);
	}


	void* readFile(void* fileVoid, int place, int elementSize, int numberElements){
		FILE* file = (FILE*) fileVoid;

		void* result = malloc(numberElements * elementSize);
		unsigned int readResult;
		readResult = fseek(file, place, SEEK_SET);
		readResult = fread(result, elementSize, numberElements, file);

		return result;
	}

	void writeFile(void* fileVoid, int place, int elementSize, int numberElement, void* value){
		FILE* file = (FILE*) fileVoid;

		unsigned int readResult;
		readResult = fseek(file, place, SEEK_SET);
		readResult = fwrite(value, elementSize, numberElement, file);
	}
	#endif

	#ifndef __LINUX_API
	void* openReadFile(void* path){
		return path; //TODO
	}

	void closeFile(void* file){

	}

	void* openWriteFile(void* path){
		return path;
	}

	void* readFile(void* fileVoid, int place, int elementSize, int numberElements){

		char* ptn = (char*) fileVoid;
		ptn = ptn + place;
		return (void*) ptn;
	}

#include <string.h>

	void writeFile(void* fileVoid, int place, int elementSize, int numberElement, void* value){
		char* dest = (char*) fileVoid;
		dest = dest + place;
		char* source = (char*) value;
		int i;
		memcpy(dest, source, elementSize * numberElement);

//		for (i=0; i<elementSize*numberElement; i++)
//			dest[i] = source[i];
	}
	#endif

	/************************************/
	//Functions for performances

	#ifdef __NIOS
	#include "altera_avalon_performance_counter.h"


	#define PERF_BEGIN(p,n) IOWR((p),(((n)*4)+1),0)
	#define PERF_END(p,n)   IOWR((p),(((n)*4)  ),0)


	void startPerformances(int id){
		PERF_BEGIN (alt_get_performance_counter_base(), id);
	}

	void stopPerformances(int id){
		PERF_END (alt_get_performance_counter_base(), id);
	}

	int getPerformances(int id){
		return 0;
	}

	#endif

	#ifdef __VLIW

	#include <platform.h> /* bare runtime services header, should be not used with OS21 */
	#define clock() bsp_timer_now()
	#define myclock_t bspclock_t


    unsigned int pmt1[4], pmt2[4];
    unsigned int pmd[4] = {0,0,0,0};

	void startPerformances(int id){
		if (id==0){
			bsp_pm_reset();
			bsp_pm_start();
			pmt1[id]=bsp_pm_clock_get();
		}
	}

	void stopPerformances(int id){
		if (id==0){
			pmt2[id]=bsp_pm_clock_get();
			pmd[id] = pmt2[id] -pmt1[id];
		}
	}

	int getPerformances(int id){
		if (id==0)
			return pmd[id];
		else
			return 0;
	}
	#endif

	#ifndef __VLIW
	#ifndef __NIOS
	void startPerformances(int id){

	}

	void stopPerformances(int id){

	}

	int getPerformances(int id){
		return 0;
	}
	#endif
	#endif

	/************************************/
	//Memcpy

	#ifdef __USE_AC
	void copyMem(void* dest, void* source, int totalSize){
		int* sourceInt = (int*) source;
		ac_int<32, false>* destInt = (ac_int<32, false>*) dest;

		int i;
		for (i=0; i<totalSize<<2; i++)
			destInt[i] = sourceInt[i];

	}
	#endif

	#ifndef __USE_AC
	void copyMem(void* dest, void* source, int totalSize){
		int* sourceInt = (int*) source;
		int* destInt = (int*) dest;

		int i;
		for (i=0; i<totalSize<<2; i++)
			destInt[i] = sourceInt[i];
	}
	#endif


