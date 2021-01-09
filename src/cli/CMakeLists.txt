# find directories with includes
include_directories(../backend)
include_directories(../common)

add_executable(assembler asm_main.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(assembler lc3core)
add_executable(simulator sim_main.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(simulator lc3core)
