#!/bin/sh -f
BENCH_BIN=../../build/gsm/src/bin/mb_toast
BENCH_OPT="-fpl"
BENCH_INP=../data/clinton.pcm
BENCH_OUT=
BENCH_ARG="${BENCH_OPT} ${BENCH_INP}"
#

echo simRISCV -f ${BENCH_BIN} -a "${BENCH_ARG}"
dbt -f ${BENCH_BIN} -a "${BENCH_ARG}"

