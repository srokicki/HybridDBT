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

TODO

## <a name="sofware"></a> First use of the software version

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