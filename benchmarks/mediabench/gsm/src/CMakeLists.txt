
include_directories(${GSM_BASE}/inc)

add_definitions(-DNeedFunctionPrototypes=1 -DSASR)

add_library(mb_gsm STATIC add.c code.c debug.c decode.c long_term.c lpc.c
	    preprocess.c rpe.c gsm_destroy.c gsm_decode.c gsm_encode.c gsm_explode.c gsm_implode.c
	    gsm_create.c gsm_print.c gsm_option.c short_term.c table.c)


add_executable(toast   toast.c toast_lin.c toast_ulaw.c toast_alaw.c toast_audio.c)
add_executable(untoast toast.c toast_lin.c toast_ulaw.c toast_alaw.c toast_audio.c)

target_link_libraries(toast   mb_gsm)
target_link_libraries(untoast mb_gsm)

set_target_properties(toast   PROPERTIES COMPILE_DEFINITIONS PROGRAM_NAME="toast")
set_target_properties(untoast PROPERTIES COMPILE_DEFINITIONS PROGRAM_NAME="untoast")

