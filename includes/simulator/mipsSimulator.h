/*
 * Simulator.h
 *
 *  Created on: 10 févr. 2016
 *      Author: simon
 */

#include <map>
#include <string>
#include <unordered_map>

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

#ifndef __NIOS

class Simulator {
public:
  std::map<int, int> memory;
  std::unordered_map<unsigned long long, int> jumps;

  Simulator(void) : memory() { jumps = {}; };

  int doSimulation(int start);
  void stw(int addr, unsigned int value);
  void sth(int addr, unsigned int value);
  void stb(int addr, unsigned int value);

  unsigned int ldw(int addr);
  unsigned int ldh(int addr);
  unsigned int ldb(int addr);

  void dumpJumps();
};

#endif
#endif /* SIMULATOR_H_ */
