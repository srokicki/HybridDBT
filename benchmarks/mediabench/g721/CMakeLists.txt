add_subdirectory(src)

add_test(NAME test_g721_encode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/encode -a "-4 -l -f ./data/clinton.pcm")
add_test(NAME test_g721_decode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/decode -a "-4 -l -f ./data/clinton.g721")
