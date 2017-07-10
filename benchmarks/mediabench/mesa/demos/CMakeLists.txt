add_executable(osdemo osdemo.c)
add_executable(mipmap mipmap.c)
add_executable(texgen texgen.c)

target_link_libraries(osdemo Mesaaux MesaGLU MesaGL m)
target_link_libraries(mipmap Mesaaux MesaGLU MesaGL m)
target_link_libraries(texgen Mesaaux MesaGLU MesaGL m)
