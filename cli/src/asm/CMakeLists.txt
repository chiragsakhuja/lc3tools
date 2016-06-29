# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/cli/src/asm)
include_directories(${PROJECT_SOURCE_DIR}/core/src)

# get all necessary files
file(GLOB CXX_FILES ${PROJECT_SOURCE_DIR}/cli/src/asm/*.cpp)
file(GLOB CXX_HEADERS ${PROJECT_SOURCE_DIR}/cli/src/asm/*.h)
file(GLOB CORE_HEADERS ${PROJECT_SOURCE_DIR}/core/src/common/*.h ${PROJECT_SOURCE_DIR}/core/src/asm/*.h ${PROJECT_SOURCE_DIR}/core/src/sim/*.h)

# grab all cpp files, except for driver (main.cpp)
set(SRCS ${CXX_FILES} ${CXX_HEADERS} ${PARSER_HEADERS})

# generate driver, and have it link against assembler library
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(assembler ${SRCS})
target_link_libraries(assembler lc3core)
