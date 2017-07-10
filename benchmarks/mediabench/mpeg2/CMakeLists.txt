add_subdirectory(src/mpeg2enc)
add_subdirectory(src/mpeg2dec)

add_test(NAME test_mpeg2_encode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/mpeg2enc/bin/mpeg2enc -a "../data/options.par ../data/out.m2v")
add_test(NAME test_mpeg2_decode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/mpeg2dec/bin/mpeg2dec -a "-b ../data/mei16v2.m2v -r -f -o ../data/tmp%d")
