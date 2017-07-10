#!/bin/sh -f
NAME=djpeg
BENCH_BIN=../../build/jpeg/jpeg-6a/bin/djpeg
BENCH_OPT="-dct int -ppm"
BENCH_INP=../data/testimg.jpg
BENCH_OUT="-outfile ../data/testout.ppm"
BENCH_ARG="${BENCH_OPT} ${BENCH_OUT} ${BENCH_INP}"
#
echo simRISCV -f ${BENCH_BIN} -a "${BENCH_ARG}" 
dbt -f ../../build/jpeg/jpeg-6a/bin/djpeg -a "-dct int -ppm -outfile ../data/testout.ppm ../data/testimg.jpg"

