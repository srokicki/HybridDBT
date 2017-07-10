#!/bin/sh -f
BENCH_BIN=../../../../build/benchmarks/mediabench/epic/src/bin/epic
BENCH_OPT="-b 25"
BENCH_INP=../data/test_image.pgm
BENCH_OUT=
BENCH_ARG="${BENCH_INP} ${BENCH_OPT} ${BENCH_OUT}"
#
dbt -f ${BENCH_BIN} -a "../data/test_image.pgm -b 25"
