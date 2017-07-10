
set(SPHERE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/sphere/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${SPHERE_INCLUDE})

add_subdirectory(sphere2/nist)

add_executable(rasta rasta.c init.c anal.c powspec.c noise_est.c mapping.c 
                audspec.c nl_audspec.c rasta_filt.c inverse_nl.c 
		post_audspec.c lpccep.c fft.c io.c fvecsubs.c svecsubs.c debug.c)
add_executable(lsqsolve lsqsolve.c mul_mdmd_md.c)

target_link_libraries(rasta sp util m)
target_link_libraries(lsqsolve m)

