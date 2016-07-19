# find directories with includes
include_directories(${PROJECT_SOURCE_DIR})

# get all necessary files
file(GLOB ASM_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/asm/*.cpp)
file(GLOB ASM_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/asm/*.h)
file(GLOB COMMON_CXX_SOURCES ${PROJECT_SOURCE_DIR}/cli/src/common/*.cpp)
file(GLOB COMMON_CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/common/*.h)

# generate driver, and have it link against assembler library
add_executable(assembler ${ASM_CXX_SOURCES} ${ASM_CXX_HEADERS} ${COMMON_CXX_SOURCES} ${COMMON_CXX_HEADERS})
target_link_libraries(assembler lc3core)
