add_subdirectory(dbt)
add_subdirectory(lib)
add_subdirectory(simulator)
add_subdirectory(isa)
add_subdirectory(transformation)
add_subdirectory(tools)
add_subdirectory(montest1)

set(SOURCE
   ${SOURCE}
   PARENT_SCOPE
)

set(EXECUTABLE
   ${EXECUTABLE}
   PARENT_SCOPE
)
