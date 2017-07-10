
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(util STATIC cfg.c comments.c ds.c err_util.c fobops.c hist.c hsgetopt.c itoa.c maldie.c mtrf.c order.c pad.c rsprintf.c sfgets.c
    statdist.c str_srch.c string.c t_or_f.c word.c)

