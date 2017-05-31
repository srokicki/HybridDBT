set(SOURCE
   ${SOURCE}
   ${CMAKE_CURRENT_SOURCE_DIR}/firstPassTranslator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/firstPassTranslatorRISCV.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/irGenerator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/irScheduler.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/optimizeBasicBlock.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/buildControlFlow.cpp   
   ${CMAKE_CURRENT_SOURCE_DIR}/reconfigureVLIW.cpp   
   ${CMAKE_CURRENT_SOURCE_DIR}/buildTraces.cpp  
   ${CMAKE_CURRENT_SOURCE_DIR}/rescheduleProcedure.cpp 
   
     
   PARENT_SCOPE
)
