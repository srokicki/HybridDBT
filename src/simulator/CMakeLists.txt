set(SOURCE
   ${SOURCE}
  ${CMAKE_CURRENT_SOURCE_DIR}/genericSimulator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/vexSimulator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/loadQueueVexSimulator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loadStoreQueue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/riscvSimulator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/mipsSimulator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/emptySimulator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shadowedRegisterFile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/riscvDecoder.cpp

   PARENT_SCOPE
)

set(SOURCE_DBT
   ${SOURCE_DBT}
  ${CMAKE_CURRENT_SOURCE_DIR}/genericSimulator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/vexSimulator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/riscvSimulator.cpp
   PARENT_SCOPE
)
