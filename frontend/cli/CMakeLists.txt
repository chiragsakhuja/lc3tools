# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/backend/src ${PROJECT_SOURCE_DIR}/frontend/common)

find_package(Threads)

# get common files
file(GLOB COMMON_CXX_SOURCES ${PROJECT_SOURCE_DIR}/frontend/common/*.cpp)
file(GLOB COMMON_CXX_HEADERS ${PROJECT_SOURCE_DIR}/frontend/common/*.h)

# do not recompile for each target
add_library(frontend_common OBJECT ${COMMON_CXX_SOURCES} ${COMMON_CXX_HEADERS})

add_executable(assembler asm_main.cpp $<TARGET_OBJECTS:frontend_common>)
target_link_libraries(assembler lc3core ${CMAKE_THREAD_LIBS_INIT})
add_executable(simulator sim_main.cpp $<TARGET_OBJECTS:frontend_common>)
target_link_libraries(simulator lc3core ${CMAKE_THREAD_LIBS_INIT})
