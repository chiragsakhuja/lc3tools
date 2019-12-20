# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/frontend/common)

find_package(Threads)

add_executable(assembler asm_main.cpp $<TARGET_OBJECTS:frontend_common>)
target_link_libraries(assembler lc3core ${CMAKE_THREAD_LIBS_INIT})
add_executable(simulator sim_main.cpp $<TARGET_OBJECTS:frontend_common>)
target_link_libraries(simulator lc3core ${CMAKE_THREAD_LIBS_INIT})

add_executable(sim_new sim_main_new.cpp $<TARGET_OBJECTS:frontend_common>)
target_link_libraries(sim_new lc3core ${CMAKE_THREAD_LIBS_INIT})
