Hybrid-DBT
=====================

[![Build Status](https://travis-ci.org/srokicki/HybridDBT.svg?branch=master)](https://travis-ci.org/srokicki/HybridDBT)

This repository contains sources of the Hybrid-DBT project. Hybrid-DBT is a hardware accelerated Dynamic Binary Translation framework which execute RISC-V or MIPS code on a VLIW. Critical part of the binary translation are done by hardware accelerators: first-pass translation which generates a first naive translation of source binaries; IR generator which decode the result of the first pass translation to generate an IR where data-flow graph is encoded; IR scheduler which read the above mentionned IR and schedule instructions in order to exploit efficiently the VLIW capacities.

## Table of Contents
+ [Building the project](#build)
+ [First use of the software version](#software)
+ [How to use the hardware version](#hardware)
+ [A quick tour of available sources](#sources)
	+ [ISA simulators](#sources_sim)
	+ [Available code transformations](#sources_transf)
	+ [Global DBT framework](#sources_dbt)
	+ [Other tools](#sources_tools)
+ [Contributors and Publications](#contributors)

## <a name="build"></a> Building the project

This part describe how to download the project sources and how to compile them. Build needs to have cmake and make installed on your computer.

First thing to do is downloading sources from the git repository:

	$ git clone https://github.com/srokicki/HybridDBT.git
	
Then create a build folder and use CMAKE to generate makefiles:
	
	$ cd HybridDBT
	$ mkdir build
	$ cd build
	$ cmake ../
	
Compile all files:
	
	$ make all

Generated binaries will be places in the folder HybridDBT/build/bin.

## <a name="sofware"></a> First use of the software version

This part will describe how to use the DBT framework in software. By "software", we mean all transformations run natively (eg. in x86) by the host computer. The execution on VLIW will be done by an instruction set simulator and return precise number of cycles needed for the execution.
This software execution can be used to measure the impact of a transformation on performance of the VLIW generated code but we cannot measure how expensive the transformation will be.

On the following, we will first see how to run the framework on existing RISC-V code (for example the elf files given in the benchmark folder). Generating a new elf file is more complex because it requires a working toolchain for RISC-V and a few modification on Newlib to work correctly on the simulator. This will be treated in details on the subpart [Generating Compatible RISC-V binaries](#riscv_compiler).

### Running the DBT framework on existing RISC-V binaries

If you want to execute the DBT framework on a given elf file (binaries.riscv), run the following command:

	$ ./build/bin/dbt [-O optLevel] [-v verboseLevel] -f binaries.riscv [-i inputStream] [-o outputStream] -- arguments
	
The parameters are the following:
* optLevel is the optimization level of the DBT process. It can take a value between 0 and two. Optimization level zero just performs the first naive translation of RISC-V binaries into VLIW, without trying to exploit ILP. Optimization level one will find blocks, generate the intermediate representation (IR) for them and use it to generate efficient binaries. Optimization level 2 will work at procedure level and try to build traces, unroll loops...
* verboseLevel can control the amount of information that will be printed during the process. Default value is zero and cause the process to print only error messages. Other values print information on more and more optimization levels. 
* binaries.riscv is the RISC-V binary file we want to execute.
* inputStream can be used to set the source of the standard input. If a file is set there, then all read on the standard input will be done on this file instead.
* outputStream can be used to set the destination of standard outputs. Same than for inputStream, if a file is set there, the output will be written in this file instead. Note that you can use several -o outputStream to allocate to the different standard output. If one is set at stderr or stdout, it will be mapped to the standard error or the standard output. 
* arguments is the list of argument to pass to the application executed.

Note that the simulator has access to the standard system calls on your computers. It can open/close files and read/write their contents.	

In addition to the DBT framework, we also provided an instruction level simulator for the RISC-V. This simulator can be used to make sure that the RISC-V binary file is compatible and should work on the DBT frameworK. This simulator shared a lot of code with the DBT framework. You can use it with the following command:

	$ ./build/bin/simRISCV [-v] -f binaries.riscv [-a "arguments"] [-i inputStream] [-o outputStream]

This simulator is built to be compatible with all binaries that the DBT framework may accept. Combined with the DBT framework we can make sure that the translation layer is correct by comparing the two simulations.

###<a name="riscv_compiler"></a> Generating Compatible RISC-V binaries

In order to compile an application and generated RISC-V binaries compatible with the DBT framework, we have to follow the instruction found on the RISC-V web page:

	$ $ git clone --recursive https://github.com/riscv/riscv-gnu-toolchain
	$ export RISCV=/path/to/install/riscv/toolchain
	
Then configure the toolchain to be compatible with the DBT framework: it needs to generate code for a RISC-V 64-bits core with the M extension. Floating point operation are done in software. You can change the prefix to choose where to install the toolchain.

	$ ./configure --prefix=/opt/riscv --with-arch=rv64im --with-abi=lp64
	$ make

	
Once the compiler is generated, you can add the folder /opt/riscv/bin to the path so that the compiler is easily used. To generate RISC-V binaries, just run the following command:

	$ riscv64-unknown-elf-gcc -std=c99 -O3 helloworld.c -o helloworld
	
## <a name="hardware"></a> How to use the hardware version

One of the specificities of Hybrid-DBT framework is to use hardware accelerators to perform critical steps in the DBT process. We identified these steps to be the first-pass translation (eg. the translation of RISC-V instructions when they are first met), the IR-generation (eg. the generation of an higher level intermediate representation of instructions) and the IR-scheduler (eg. the instruction scheduling which transforms the abovementionned IR into VLIW binaries).

These three accelerators are generated using High-Level Synthesis (HLS). The VHDL files are stored in the architecture/vhdl folder.

The platform we used for prototyping us built upon Nios II processor from Altera. Each of the abovementionned accelerators are called ny the Nios II as custom instructions. The tool QSys was used to describe the system. Folder architecture/vhdl also contains VHDL wrapper and TCL files for instantiating accelerators in QSys. If you simply modify the PATH of the QSys tool, you'll be able to add the three accelerators in your system.

Once the platform is defined and compiled using Quartus, use the Nios II tools to build the project for the platform. For this, you'll have to generate a BSP corresponding to your platform and to build the project using the "-D __NIOS" flag.


## <a name="sources"></a> A quick tour of available sources

In this section we will quickly present how sources of the project are organized and where you'll find most interesting parts.

### <a name="sources_sim"></a> ISA simulators

An important part of the framework is the code for instruction set simulators. We currently implemented simulators for three ISA: **RISC-V**, **MIPS** and **VEX**. Those simulator are built upon the same principle: you fill instruction into main memory and start the execution at a given PC. The simulator will decode and execute each instruction maintaining an array which represent the register file.

The simulator for VEX processor is a bit special because it aims at being compiled by an HLS tool (for us Catapult) in order to generate VHDL. The VLIW we use in the hardware platform is derived from this code.

Declaration of ISA instructions can be found in includes/ISA. Definition of simulators are in includes/simulator.
Finally folder src/simulator contains sources for different simulators.

**Note:** Simulator can also be used in standalone mode from a dedicated executable. See subpart on other tools for more details. 

### <a name="sources_transf"></a> Available code transformations

Transformations implemented in the framework are store in the folder src/transformation. We can separate them into two categories: 
 - Transformations done by a hardware accelerator
 - Transformations done in software

 
The first category contains currently three transformations: the **firstPassTranslation**, the **IRBuilder** and the IRScheduler. Their role is to efficiently handle the first steps of the DBT process. 
 
 **FirstPassTranslator** will generate a naive translation of source binaries into VLIW binaries. In order to reduce its overhead, it will not try to exploit ILP and will never schedule more than one instruction per cycle. Currently two implementation of the firstPassTranslator exists: one for MIPS ISA and one for RISC-V ISA. The second one is the most recent one and is the one tested after updates. The other one is currently deprecated.
 
 **IRBuilder** will read a piece of VLIW binaries and analyze dependencies to build a more precise reprensentation of the code. In this IR, all data and control dependencies are directly encoded which make the further optimizations on code easier to perform.
 
 **IRScheduler** will read the above generated IR and schedule instruction on different execution units of the VLIW. This accelerator is designed to be reconfigurable and can handle VLIW configuration going from 2 to 8 issues and having 8 to 64 registers in its register file.
 
 As we said before, these three transformation are performed by hardware accelerators in order to reduce the cost of first steps of the DBT process. The code contained in the repository is the C code used to generate the accelerators, through High Level Synthesis. It has been designed to generate efficient hardware.
 
 
 Other transformations are upper level transformations for further code optimizations.
 

### <a name="sources_dbt"></a> Global DBT framework

TODO

### <a name="sources_tools"></a> Other tools

TODO

## <a name="contributors"></a> Contributors and Publications

Project has been developed at University of Rennes, France (INRIA/IRISA). 
Main contributors are <a href="http://people.irisa.fr/Simon.Rokicki/index.html">Simon Rokicki</a>, Arthur Blanleuil, Erven Rohou and Steven Derrien.

The framework has been presented at DATE'17. Document can be found <a href="https://hal.archives-ouvertes.fr/hal-01423639v1">here</a>. 
Same framework is also used for a paper at DATE'18 named <a href="https://hal.archives-ouvertes.fr/hal-01653110v2"> Supporting Runtime Reconfigurable VLIWs Cores Through Dynamic Binary Translation</a>