add_subdirectory(src)


add_test(NAME test_adpcm_encode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/rawcaudio -i ../data/clinton.pcm -o ../results/out.adpcm) 
add_test(NAME test_adpcm_decode WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/rawdaudio -i ../data/clinton.adpcm -o ../results/out.pcm) 
