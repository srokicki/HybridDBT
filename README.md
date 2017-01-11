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

	$git clone https://github.com/srokicki/HybridDBT.git
	
Then create a build folder and use CMAKE to generate makefiles:
	
	$cd HybridDBT
	$mkdir build
	$cd build
	$cmake ../
	
Compile all files:
	
	$make all

Generated binaries will be places in the folder HybridDBT/build/bin.

## <a name="sofware"></a> First use of the software version

This part will describe how to use the DBT framework in software. By "software", we mean all transformations run natively (eg. in x86) by the host computer. The execution on VLIW will be done by an instruction set simulator and return precise number of cycles needed for the execution.
This software execution can be used to measure the impact of a transformation on performance of the VLIW generated code but we cannot measure how expensive the transformation will be.

On the following, we will first see how to run the framework on existing RISC-V code (for example the elf files given in the benchmark folder). Generating a new elf file is more complex because it requires a working toolchain for RISC-V and a few modification on Newlib to work correctly on the simulator. This will be treated in details on the subpart [Generating Compatible RISC-V binaries](#riscv_compiler).

### Running the DBT framework on existing RISC-V binaries

If you want to execute the DBT framework on a given elf file (for example benchmarks/simple/dct/dct), run the following command:

	$./build/bin/dbt benchmarks/simple/dct/dct
	
Debug messages from the DBT framework are printed on the standard error while application print from the executed code are printed on the standard output.

###<a name="riscv_compiler"></a> Generating Compatible RISC-V binaries

In order to compile an application and generated RISC-V binaries compatible with the DBT framework, we have to follow the instruction found on the RISC-V web page:

	$ git clone https://github.com/riscv/riscv-tools.git
	$ cd riscv-tools
	$ git submodule update --init --recursive
	$ export RISCV=/path/to/install/riscv/toolchain
	
Here, instead of running the build.sh script as it is presented on the official web page, we will run another script that generate 32-bit binaries without support for hardware floating point.

TODO

## <a name="hardware"></a> How to use the hardware version

TODO

## <a name="sources"></a> A quick tour of available sources

TODO

### <a name="sources_sim"></a> ISA simulators

TODO

### <a name="sources_transf"></a> Available code transformations

TODO

### <a name="sources_dbt"></a> Global DBT framework

TODO

### <a name="sources_tools"></a> Other tools

TODO

## <a name="contributors"></a> Contributors and Publications

Project has been developed at University of Rennes, France (INRIA/IRISA). 
Main contributors are <a href="http://people.irisa.fr/Simon.Rokicki/index.html">Simon Rokicki</a>, Erven Rohou and Steven Derrien.

The framework has been presented at DATE'17. Document can be found <a href="https://hal.archives-ouvertes.fr/hal-01423639v1">here</a>. 