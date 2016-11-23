/************************************
 * Functions to handle different environments (Nios Soft / Nios Hard / Linux)
 ************************************/
#include "types.h"

/************************************/
//Functions to accede bytecode and binaries

void* openReadFile(void* path);
void closeFile(void* file);
void* openWriteFile(void* path);
void* readFile(void* fileVoid, int place, int elementSize, int numberElements);
void writeFile(void* fileVoid, int place, int elementSize, int numberElement, void* value);

/************************************/
//Functions for performances

void startPerformances(int id);

void stopPerformances(int id);

int getPerformances(int id);

/************************************/
//Memcpy

void copyMem(void* dest, void* source, int totalSize);

/************************************/
//Scheduling


