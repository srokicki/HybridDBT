add_subdirectory(dbt)
add_subdirectory(frontend)
add_subdirectory(lib)
add_subdirectory(simulator)
add_subdirectory(isa)
add_subdirectory(transformation)
add_subdirectory(tools)

set(SOURCE
   ${SOURCE}
   PARENT_SCOPE
)

set(EXECUTABLE
   ${EXECUTABLE}
   PARENT_SCOPE
)
