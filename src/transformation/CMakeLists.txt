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
   ${CMAKE_CURRENT_SOURCE_DIR}/cgraScheduler.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/localRegisterAnalysis.cpp
   
     
   PARENT_SCOPE
)


set(SOURCE_DBT
   ${SOURCE_DBT}
   ${CMAKE_CURRENT_SOURCE_DIR}/optimizeBasicBlock.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/buildControlFlow.cpp   
   ${CMAKE_CURRENT_SOURCE_DIR}/reconfigureVLIW.cpp   
   ${CMAKE_CURRENT_SOURCE_DIR}/buildTraces.cpp  
   ${CMAKE_CURRENT_SOURCE_DIR}/rescheduleProcedure.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/memoryDisambiguation.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/cgraScheduler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/localRegisterAnalysis.cpp
   
     
   PARENT_SCOPE
)
