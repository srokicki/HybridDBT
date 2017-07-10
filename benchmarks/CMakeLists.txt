cmake_minimum_required (VERSION 3.5)

unset(CMAKE_C_COMPILER PARENT_SCOPE)
unset(CMAKE_CXX_COMPILER PARENT_SCOPE)
unset(CMAKE_C_FLAGS PARENT_SCOPE)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY PARENT_SCOPE)



set(CMAKE_C_COMPILER riscv64-unknown-elf-gcc)
set(CMAKE_CXX_COMPILER riscv64-unknown-elf-g++)
set(CMAKE_C_FLAGS "-O3 -static")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

enable_testing()


add_subdirectory(mediabench)

