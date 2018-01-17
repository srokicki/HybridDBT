/*
 * genericSimulator.h
 *
 *  Created on: 25 avr. 2017
 *      Author: simon
 */

#ifndef INCLUDES_SIMULATOR_GENERICSIMULATOR_H_
#define INCLUDES_SIMULATOR_GENERICSIMULATOR_H_

#ifndef __NIOS

#include <map>
#include <lib/ac_int.h>

/*********************************************************
 *    Definition of system calls IDs
 *
 * These IDs are from the newlib implementation of system
 * calls inside the RISCV toolchain.
 *********************************************************/
#define SYS_exit 93
#define SYS_exit_group 94
#define SYS_getpid 172
#define SYS_kill 129
#define SYS_read 63
#define SYS_write 64
#define SYS_open 1024
#define SYS_openat 56
#define SYS_close 57
#define SYS_lseek 62
#define SYS_brk 214
#define SYS_link 1025
#define SYS_unlink 1026
#define SYS_mkdir 1030
#define SYS_chdir 49
#define SYS_getcwd 17
#define SYS_stat 1038
#define SYS_fstat 80
#define SYS_lstat 1039
#define SYS_fstatat 79
#define SYS_access 1033
#define SYS_faccessat 48
#define SYS_pread 67
#define SYS_pwrite 68
#define SYS_uname 160
#define SYS_getuid 174
#define SYS_geteuid 175
#define SYS_getgid 176
#define SYS_getegid 177
#define SYS_mmap 222
#define SYS_munmap 215
#define SYS_mremap 216
#define SYS_time 1062
#define SYS_getmainvars 2011
#define SYS_rt_sigaction 134
#define SYS_writev 66
#define SYS_gettimeofday 169
#define SYS_times 153
#define SYS_fcntl 25
#define SYS_getdents 61
#define SYS_dup 23

/*********************************************************
 * 	Definition of the GenericSimulator class
 *
 * 	This class will contain all methods common to all ISA
 * 	simulators. It includes memory accesses and syscalls
 *
 *********************************************************/


class GenericSimulator {
public:

GenericSimulator(void) : memory(){this->debugLevel = 0;};

int debugLevel = 0;
int stop = 0;

std::map<ac_int<64, false>, ac_int<8, true>> memory;
ac_int<64, true> REG[64];
float regf[64];
void initialize(int argc, char* argv[]);


//********************************************************
//Memory interfaces

void stb(ac_int<64, false> addr, ac_int<8, true> value);
void sth(ac_int<64, false> addr, ac_int<16, true> value);
void stw(ac_int<64, false> addr, ac_int<32, true> value);
void std(ac_int<64, false> addr, ac_int<64, true> value);

ac_int<8, true> ldb(ac_int<64, false> addr);
ac_int<16, true> ldh(ac_int<64, false> addr);
ac_int<32, true> ldw(ac_int<64, false> addr);
ac_int<64, true> ldd(ac_int<64, false> addr);

//********************************************************
//System calls

std::map<ac_int<16, true>, FILE*> fileMap;
FILE **inStreams, **outStreams;
int nbInStreams, nbOutStreams;
unsigned int heapAddress;

ac_int<64, false> solveSyscall(ac_int<64, false> syscallId, ac_int<64, false> arg1, ac_int<64, false> arg2, ac_int<64, false> arg3, ac_int<64, false> arg4);

ac_int<64, false> doRead(ac_int<64, false> file, ac_int<64, false> bufferAddr, ac_int<64, false> size);
ac_int<64, false> doWrite(ac_int<64, false> file, ac_int<64, false> bufferAddr, ac_int<64, false> size);
ac_int<64, false> doOpen(ac_int<64, false> name, ac_int<64, false> flags, ac_int<64, false> mode);
ac_int<64, false> doOpenat(ac_int<64, false> dir, ac_int<64, false> name, ac_int<64, false> flags, ac_int<64, false> mode);
ac_int<64, true> doLseek(ac_int<64, false> file, ac_int<64, false> ptr, ac_int<64, false> dir);
ac_int<64, false> doClose(ac_int<64, false> file);
ac_int<64, false> doStat(ac_int<64, false> filename, ac_int<64, false> ptr);
ac_int<64, false> doSbrk(ac_int<64, false> value);
ac_int<64, false> doGettimeofday(ac_int<64, false> timeValPtr);
ac_int<64, false> doUnlink(ac_int<64, false> path);

};

#endif

#endif /* INCLUDES_SIMULATOR_GENERICSIMULATOR_H_ */
