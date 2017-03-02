#No non executable sources here

set(EXECUTABLE
   ${EXECUTABLE}
   ${CMAKE_CURRENT_SOURCE_DIR}/simRISCV.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/simMIPS.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dumpBinaryCode.cpp
   PARENT_SCOPE
)
