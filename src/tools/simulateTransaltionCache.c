#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static FILE* fp_trace;
int* sizeMap;
long long int* addrMap;

#define TRANSLATION_CACHE_SIZE 4096

int translationCache[TRANSLATION_CACHE_SIZE];
int placeToRead = 0, placeToWrite = 0, nbElement = 0, sizeAvailable = TRANSLATION_CACHE_SIZE;

int nbTranslatedInstr = 0;

void initMap()
{

  sizeMap = (int*)malloc(65536 * sizeof(int));
  addrMap = (long long int*)malloc(65536 * sizeof(long long int));

  FILE* symbols = fopen("./sym.dump", "r");

  long long int addr;
  int size;
  int index = 0;
  int n     = fscanf(symbols, "%llx, %d\n", &addr, &size);
  printf("rest %d\n", n);
  while (n == 2) {
    printf("adding %llx %d\n", addr, size);
    sizeMap[index] = size;
    addrMap[index] = addr;
    index++;
    n = fscanf(symbols, "%llx, %d\n", &addr, &size);
  }
  fclose(symbols);
}

void __attribute__((constructor)) trace_begin(void)
{
  initMap();
  fp_trace = fopen("trace.out", "w");
}

void __attribute__((destructor)) trace_end(void)
{
  if (fp_trace != NULL) {
    fclose(fp_trace);
  }

  printf("Tracing found that %d instruction has been translated with a cache of size %d\n", nbTranslatedInstr,
         TRANSLATION_CACHE_SIZE);
}

void __cyg_profile_func_enter(void* func, void* caller)
{
  if (fp_trace != NULL) {

    int index = -1;
    for (int oneAddr = 0; oneAddr < 137; oneAddr++) {
      if (addrMap[oneAddr] == (long long int)func) {
        index = oneAddr;
        break;
      }
    }

    if (index != -1) {
      // We found the symbol
      printf("test\n");
      if (sizeMap[index] > 0) {

        // We check if it is in the translation cache
        char isPresent = 0;
        int oneElement = placeToRead;
        while (!isPresent && oneElement < placeToWrite) {
          if (translationCache[oneElement] == index)
            isPresent = 1;
          else
            oneElement = (oneElement + 1) % TRANSLATION_CACHE_SIZE;
        }
        if (!isPresent) {
          if (sizeMap[index] <= sizeAvailable) {
            sizeAvailable -= sizeMap[index];
            translationCache[placeToWrite] = index;
            placeToWrite                   = (placeToWrite + 1) % TRANSLATION_CACHE_SIZE;
            nbTranslatedInstr += sizeMap[index] >> 2;
          } else {
            while (placeToRead < placeToWrite && sizeAvailable < sizeMap[index]) {
              int indexToRemove = translationCache[placeToRead];
              int sizeToRemove  = sizeMap[indexToRemove];
              sizeAvailable += sizeToRemove;
              placeToRead = (placeToRead + 1) % TRANSLATION_CACHE_SIZE;
            }

            if (sizeAvailable < sizeMap[index]) {
              printf("Error : cache is too small to handle one of the procedure\n");
              exit(-1);
            }

            sizeAvailable += sizeMap[index];
            translationCache[placeToWrite] = index;
            placeToWrite                   = (placeToWrite + 1) % TRANSLATION_CACHE_SIZE;
            nbTranslatedInstr += sizeMap[index] >> 2;
          }
        }
      }
    }
  }
}

void __cyg_profile_func_exit(void* func, void* caller) {}
