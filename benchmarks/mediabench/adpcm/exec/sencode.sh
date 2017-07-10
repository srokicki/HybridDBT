#!/bin/sh -f
BENCHMARK=../../build/adpcm/src/bin/rawcaudio
INPUT=../data/clinton.pcm
OUTPUT=../results/out.adpcm
#
echo simRISCV ${BENCHMARK} -i ${INPUT} -o ${OUTPUT}
dbt -f ${BENCHMARK} -i ${INPUT} -o ${OUTPUT}
