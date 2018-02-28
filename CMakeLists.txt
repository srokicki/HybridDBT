cmake_minimum_required (VERSION 3.0.2)


enable_testing()


project (HybridDBT)

 
# add the binary tree to the search path for include files
include_directories(./includes)

#-fsanitize=address
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ggdb -std=c++11 -D__LINUX_API -D__USE_AC -D AC_NOT_USING_INTN -pthread ")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)
set (EXECUTABLE  "")

add_subdirectory(src)

#add_executable(dumpBinaryBytecode ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/dumpBinaryBytecode.cpp)
#add_executable(dumpBinaryCode ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/dumpBinaryCode.cpp)
#add_executable(dumpResultsForAcceleratedSimulation ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/dumpResultsForAcceleratedSimulation.cpp)
#add_executable(readBytecode ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/readBytecode.cpp)
#add_executable(dbt ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/src/dbt/dbt.cpp)

foreach(oneExecutable ${EXECUTABLE})
	get_filename_component(execName ${oneExecutable} NAME_WE)

  if (execName MATCHES "dbt")
    add_executable(dbtSw ${SOURCE} ${oneExecutable})
    set_target_properties(dbtSw PROPERTIES COMPILE_DEFINITIONS "__SW")
    
    add_executable(dbtHwSim ${SOURCE} ${oneExecutable})
    set_target_properties(dbtHwSim PROPERTIES COMPILE_DEFINITIONS "__HW_SIM")
    
    add_executable(dbt ${SOURCE} ${oneExecutable})
    set_target_properties(dbt PROPERTIES COMPILE_DEFINITIONS "__HW_SIM")
  elseif(execName MATCHES "cgra")
      add_executable(cgra ${SOURCE} ${oneExecutable})
      target_link_libraries(cgra
          /home/ablanleu/Documents/utils/pcm/cpucounters.o
          /home/ablanleu/Documents/utils/pcm/msr.o
          /home/ablanleu/Documents/utils/pcm/pci.o
          /home/ablanleu/Documents/utils/pcm/utils.o
          /home/ablanleu/Documents/utils/pcm/client_bw.o)
      include_directories(/home/ablanleu/Documents/utils/pcm/)

      set_target_properties(cgra PROPERTIES COMPILE_DEFINITIONS "__HW_SIM")
  else()
	  add_executable(${execName} ${SOURCE} ${oneExecutable})
	  set_target_properties(${execName} PROPERTIES COMPILE_DEFINITIONS "__SW")
	  
  endif()

endforeach(oneExecutable)




