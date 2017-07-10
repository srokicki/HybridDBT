add_executable(mpeg2enc mpeg2enc.c conform.c putseq.c putpic.c puthdr.c putmpg.c putvlc.c putbits.c motion.c predict.c readpic.c writepic.c
    transfrm.c fdctref.c idct.c quantize.c ratectl.c stats.c)

target_link_libraries(mpeg2enc m)

