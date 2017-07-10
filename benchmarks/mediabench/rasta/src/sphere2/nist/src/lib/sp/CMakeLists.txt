
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(sp STATIC alaw.c array.c dif_wavf.c dupfinfo.c exit.c fixio.c h_fix.c license.c lpc.c nat_bord.c poly.c sdatamod.c shorten.c
    shpack_s.c shpacki2.c shpackio.c sp_ntoh.c sp_utils.c spalloc.c spchksum.c spclose.c speof.c sperror.c sphere.c sphio.c spinput.c sploadf.c
    spopen.c spoutput.c spread.c sprewind.c sprstat.c spseek.c sptell.c sptemp.c sputils2.c spvers.c spwrite.c stdfield.c ulaw.c vario.c
    verbose.c wavedit.c wavpack.c)

