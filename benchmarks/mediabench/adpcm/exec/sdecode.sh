#!/bin/sh -f
BENCHMARK=../../build/adpcm/src/bin/rawdaudio
INPUT=../data/clinton.adpcm
OUTPUT=../results/out.pcm
#
dbt -f ${BENCHMARK} -i ${INPUT} -o ${OUTPUT}
