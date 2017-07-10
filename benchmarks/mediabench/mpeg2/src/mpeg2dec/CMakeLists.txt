add_executable(mpeg2dec mpeg2dec.c getpic.c motion.c getvlc.c gethdr.c getblk.c getbits.c store.c recon.c spatscal.c idct.c idctref.c
    display.c systems.c subspic.c verify.c)

target_link_libraries(mpeg2dec m)
