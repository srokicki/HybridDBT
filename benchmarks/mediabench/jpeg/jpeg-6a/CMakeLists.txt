
# Put here the object file name for the correct system-dependent memory
# manager file.  For Unix this is usually jmemnobs.c, but you may want
# to use jmemansi.c or jmemname.c if you have limited swap space.
set(SYSDEPMEM jmemnobs.c)

set(COMOBJECTS jcomapi.c jutils.c jerror.c jmemmgr.c ${SYSDEPMEM})
# compression library object files
set(CLIBOBJECTS jcapimin.c jcapistd.c jctrans.c jcparam.c jdatadst.c jcinit.c 
        jcmaster.c jcmarker.c jcmainct.c jcprepct.c jccoefct.c jccolor.c 
        jcsample.c jchuff.c jcphuff.c jcdctmgr.c jfdctfst.c jfdctflt.c 
        jfdctint.c)
# decompression library object files
set(DLIBOBJECTS jdapimin.c jdapistd.c jdtrans.c jdatasrc.c jdmaster.c 
        jdinput.c jdmarker.c jdhuff.c jdphuff.c jdmainct.c jdcoefct.c 
        jdpostct.c jddctmgr.c jidctfst.c jidctflt.c jidctint.c jidctred.c 
        jdsample.c jdcolor.c jquant1.c jquant2.c jdmerge.c)

add_library(jpeg STATIC ${CLIBOBJECTS} ${DLIBOBJECTS} ${COMOBJECTS})

add_executable(cjpeg cjpeg.c rdppm.c rdgif.c rdtarga.c rdrle.c rdbmp.c rdswitch.c cdjpeg.c)
add_executable(djpeg djpeg.c wrppm.c wrgif.c wrtarga.c wrrle.c wrbmp.c rdcolmap.c cdjpeg.c)
add_executable(jpegtran jpegtran.c rdswitch.c cdjpeg.c)
add_executable(rdjpgcom rdjpgcom.c)
add_executable(wrjpgcom wrjpgcom.c)
add_executable(ansi2knr ansi2knr.c)

target_link_libraries(cjpeg jpeg)
target_link_libraries(djpeg jpeg)
target_link_libraries(jpegtran jpeg)
