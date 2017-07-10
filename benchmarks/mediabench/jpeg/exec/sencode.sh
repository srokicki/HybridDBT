#!/bin/sh -f
BENCH_BIN=../../build/jpeg/jpeg-6a/bin/cjpeg
BENCH_OPT="-dct int -progressive -opt"
BENCH_INP=../data/testimg.ppm
BENCH_OUT="-outfile ../data/testout.jpeg"
BENCH_ARG="${BENCH_OPT} ${BENCH_OUT} ${BENCH_INP}"
#
echo simRISCV -f ${BENCH_BIN} -a "${BENCH_ARG}"
dbt -f ${BENCH_BIN} -a "${BENCH_ARG}" 
