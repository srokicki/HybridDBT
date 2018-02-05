set(SOURCE
   ${SOURCE}
  ${CMAKE_CURRENT_SOURCE_DIR}/genericSimulator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/vexSimulator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/vexTraceSimulator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/riscvSimulator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/mipsSimulator.cpp   
   PARENT_SCOPE
)

set(SOURCE_DBT
   ${SOURCE_DBT}
  ${CMAKE_CURRENT_SOURCE_DIR}/genericSimulator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/vexSimulator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/riscvSimulator.cpp
   PARENT_SCOPE
)
