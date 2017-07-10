add_subdirectory(src)

add_test(NAME test_epic_decode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/unepic -a "./data/test.image.pgm.E") 
add_test(NAME test_epic_encode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/epic -a "./data/test_image.pgm") 
