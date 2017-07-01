# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/core/src)

# get all necessary files
file(GLOB SIM_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/asm/*.cpp)
file(GLOB SIM_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/asm/*.h)
file(GLOB SIM_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/sim/*.cpp)
file(GLOB SIM_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/sim/*.h)
file(GLOB COMMON_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/common/*.cpp)
file(GLOB COMMON_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/common/*.h)

# generate driver, and have it link against assembler library
add_executable(assembler ${SIM_CXX_SOURCES} ${SIM_CXX_HEADERS} ${COMMON_CXX_SOURCES} ${COMMON_CXX_HEADERS})
target_link_libraries(assembler lc3core)
add_executable(sim ${SIM_CXX_SOURCES} ${SIM_CXX_HEADERS} ${COMMON_CXX_SOURCES} ${COMMON_CXX_HEADERS})
target_link_libraries(sim lc3core)
