set(CMAKE_CXX_FLAGS "-std=c++11")

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser lc3.y ${PROJECT_BINARY_DIR}/src/as/parser.cpp)
FLEX_TARGET(Scanner lc3.l ${PROJECT_BINARY_DIR}/src/as/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

file(GLOB CXX_AS_FILES ${PROJECT_SOURCE_DIR}/src/as/*.cpp)
file(GLOB CXX_UTILS_FILES ${PROJECT_SOURCE_DIR}/src/utils/*.cpp)
set(PROJECT_SRCS ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS} ${CXX_AS_FILES} ${CXX_UTILS_FILES})

include_directories(${PROJECT_SOURCE_DIR}/src/as)
include_directories(${PROJECT_BINARY_DIR}/src/as)
link_directories(${PROJECT_SOURCE_DIR}/src/utils)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(lc3as ${PROJECT_SRCS})
