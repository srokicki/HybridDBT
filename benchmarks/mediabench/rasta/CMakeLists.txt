add_subdirectory(src)

add_test(NAME test_rasta WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/src/bin/rasta -a "-z -A -J -S 8000 -n 12 -f map_weights.dat" -i ex5_c1.wav -o ex5.asc -o stderr -o rasta.shade) 


