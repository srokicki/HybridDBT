add_subdirectory(firstPassTranslation)
add_subdirectory(irGeneration)
add_subdirectory(irScheduling)

set(SOURCE
   ${SOURCE}
   ${CMAKE_CURRENT_SOURCE_DIR}/optimizeBasicBlock.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/buildControlFlow.cpp   
   ${CMAKE_CURRENT_SOURCE_DIR}/reconfigureVLIW.cpp   
   ${CMAKE_CURRENT_SOURCE_DIR}/buildTraces.cpp  
   ${CMAKE_CURRENT_SOURCE_DIR}/rescheduleProcedure.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/memoryDisambiguation.cpp 
   
     
   PARENT_SCOPE
)


