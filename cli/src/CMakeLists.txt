# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/core/src ${PROJECT_SOURCE_DIR}/cli/src/common)

find_package(Threads)

# get all necessary files
file(GLOB ASM_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/asm/*.cpp)
file(GLOB ASM_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/asm/*.h)
file(GLOB SIM_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/sim/*.cpp)
file(GLOB SIM_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/sim/*.h)
file(GLOB COMMON_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/common/*.cpp)
file(GLOB COMMON_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/common/*.h)

add_executable(assembler ${ASM_CXX_SOURCES} ${ASM_CXX_HEADERS} ${COMMON_CXX_SOURCES} ${COMMON_CXX_HEADERS})
target_link_libraries(assembler lc3core ${CMAKE_THREAD_LIBS_INIT})
add_executable(simulator ${SIM_CXX_SOURCES} ${SIM_CXX_HEADERS} ${COMMON_CXX_SOURCES} ${COMMON_CXX_HEADERS})
target_link_libraries(simulator lc3core ${CMAKE_THREAD_LIBS_INIT})
