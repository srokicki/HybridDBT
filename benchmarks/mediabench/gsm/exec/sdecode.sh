#!/bin/sh -f
BENCH_BIN=../../build/gsm/src/bin/untoast
BENCH_OPT="-fpl"
BENCH_INP=../data/clinton.pcm.run.gsm
BENCH_OUT=../data/clinton.pcm.run
BENCH_ARG="${BENCH_OPT} ${BENCH_INP}"
#

echo simRISCV -f ${BENCH_BIN} -a "${BENCH_ARG}" 
dbt -f ${BENCH_BIN} -a "${BENCH_ARG}" 


