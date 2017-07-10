
set(GSM_BASE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src)

add_test(NAME test_gsm_encode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/toast -a "-fpl ./data/clinton.pcm") 
add_test(NAME test_gsm_decode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/untoast -a "-fpl ./data/clinton.pcm.run.gsm") 
