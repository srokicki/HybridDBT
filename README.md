Hybrid-DBT
=====================

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

	$ ./build/bin/dbt [-O 1|-v] binaries.riscv [-a "arguments"]
	
When running on verbose mode (-v flag), debug messages from the DBT framework are printed on the standard error while all application messages are printed on the standard output. Simulator will also have access to all standard system calls through the simulator. It can for example, read/write files in your file system.

We also provided an instruction level simulator for the RISC-V. You can use it with the following command:

	$ ./build/bin/simRISCV [-v] binaries.riscv [-a "arguments"]

This simulator is built to be compatible with all binaries that the DBT framework may accept. Combined with the DBT framework we can make sure that the translation layer is correct by comparing the two simulations.

###<a name="riscv_compiler"></a> Generating Compatible RISC-V binaries

In order to compile an application and generated RISC-V binaries compatible with the DBT framework, we have to follow the instruction found on the RISC-V web page:

	$ git clone https://github.com/riscv/riscv-tools.git
	$ cd riscv-tools
	$ git submodule update --init --recursive
	$ export RISCV=/path/to/install/riscv/toolchain
	
Then run the script provided to build the compiler:

	$ ./build.sh
	
Once the compiler is done, you can at the folder $RISCV/bin to the path so that the compiler is easily used. To generate code with soft-float, just run the following command:

	$ riscv64-unknown-elf-gcc -std=c99 -msoft-float -march=RV32IM -O3 helloworld.c -o helloworld
	
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
Main contributors are <a href="http://people.irisa.fr/Simon.Rokicki/index.html">Simon Rokicki</a>, Erven Rohou and Steven Derrien.

The framework has been presented at DATE'17. Document can be found <a href="https://hal.archives-ouvertes.fr/hal-01423639v1">here</a>. 
