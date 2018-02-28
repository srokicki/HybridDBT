set(SOURCE
   ${SOURCE}
   ${CMAKE_CURRENT_SOURCE_DIR}/elfFile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/debugFunctions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/endianness.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/config.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/traceQueue.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/threadedDebug.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dbtProfiling.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tikzDrawer.cpp
   PARENT_SCOPE
)


set(SOURCE_DBT
   ${SOURCE_DBT}
   ${CMAKE_CURRENT_SOURCE_DIR}/dbtProfiling.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/endianness.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
   PARENT_SCOPE
)
