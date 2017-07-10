include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(src-aux)
add_subdirectory(src-glu)
add_subdirectory(demos)


add_test(NAME test_mesa_mipmap WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/demos/bin/mipmap)
add_test(NAME test_mesa_osdemo WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/demos/bin/osdemo)
add_test(NAME test_mesa_texgen WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ COMMAND dbt -f ${CMAKE_CURRENT_BINARY_DIR}/demos/bin/texgen)
