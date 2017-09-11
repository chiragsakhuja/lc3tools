# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/core/src ${PROJECT_SOURCE_DIR}/wrapper ${PROJECT_SOURCE_DIR}/cli/src/common)

find_package(Threads)

# get common files
file(GLOB WRAPPER_CXX_SOURCES ${PROJECT_SOURCE_DIR}/wrapper/*.cpp)
file(GLOB WRAPPER_CXX_HEADERS ${PROJECT_SOURCE_DIR}/wrapper/*.h)
file(GLOB COMMON_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/common/*.cpp)
file(GLOB COMMON_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/common/*.h)

add_library(cli_common OBJECT ${WRAPPER_CXX_SOURCES} ${WRAPPER_CXX_HEADERS} ${COMMON_CXX_SOURCES} ${COMMON_CXX_HEADERS})

add_executable(assembler asm_main.cpp $<TARGET_OBJECTS:cli_common>)
target_link_libraries(assembler lc3core ${CMAKE_THREAD_LIBS_INIT})
add_executable(simulator sim_main.cpp $<TARGET_OBJECTS:cli_common>)
target_link_libraries(simulator lc3core ${CMAKE_THREAD_LIBS_INIT})
