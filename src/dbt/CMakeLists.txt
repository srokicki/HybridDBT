set(SOURCE
  	${SOURCE}
   	${CMAKE_CURRENT_SOURCE_DIR}/dbtPlateform.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/initCode.cpp
  	${CMAKE_CURRENT_SOURCE_DIR}/insertions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/profiling.cpp

   PARENT_SCOPE
)

set(SOURCE_DBT
   ${SOURCE_DBT}
   	${CMAKE_CURRENT_SOURCE_DIR}/dbtPlateform.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/initCode.cpp
  	${CMAKE_CURRENT_SOURCE_DIR}/insertions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/profiling.cpp

   PARENT_SCOPE
)

set(EXECUTABLE
   ${EXECUTABLE}
   ${CMAKE_CURRENT_SOURCE_DIR}/dbt.cpp

   PARENT_SCOPE
)
