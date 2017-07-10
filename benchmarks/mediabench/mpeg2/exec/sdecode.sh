#!/bin/sh -f
NAME=mpeg2decode
BENCH_BIN=../../build/mpeg2/src/mpeg2dec/bin/mpeg2dec
BENCH_OPT="-r -f -o"
BENCH_INP="-b ../data/mei16v2.m2v"
BENCH_OUT="../data/tmp%d"
BENCH_ARG="${BENCH_INP} ${BENCH_OPT} ${BENCH_OUT}"
#
echo simRISCV -f ${BENCH_BIN} -a "${BENCH_ARG}"
simRISCV -f ${BENCH_BIN} -a "${BENCH_ARG}"

