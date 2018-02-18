# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/backend/src ${PROJECT_SOURCE_DIR}/frontend/common)

find_package(Threads)

add_executable(assembler asm_main.cpp common.cpp $<TARGET_OBJECTS:frontend_common>)
target_link_libraries(assembler lc3core_static ${CMAKE_THREAD_LIBS_INIT})
add_executable(simulator sim_main.cpp common.cpp $<TARGET_OBJECTS:frontend_common>)
target_link_libraries(simulator lc3core_static ${CMAKE_THREAD_LIBS_INIT})
