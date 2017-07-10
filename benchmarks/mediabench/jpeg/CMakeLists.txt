add_subdirectory(jpeg-6a)

add_test(NAME test_jpeg_encode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/jpeg-6a/bin/cjpeg -a "-dct int -progressive -opt -outfile ./data/testout.jpeg ./data/testimg.ppm") 
add_test(NAME test_jpeg_decode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/jpeg-6a/bin/djpeg -a "-dct int -ppm -outfile ./data/testout.ppm ./data/testimg.jpg") 
