#!/bin/sh -f
BENCH_BIN=../../../../build/benchmarks/mediabench/epic/src/bin/unepic
BENCH_OPT=
BENCH_INP=../data/test.image.pgm.E
BENCH_OUT=
BENCH_ARG="${BENCH_INP} ${BENCH_OPT} ${BENCH_OUT}"
#
simRISCV -f ${BENCH_BIN} -a "${BENCH_ARG}"
