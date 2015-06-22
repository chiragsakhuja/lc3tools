# used findFLEX
find_package(BISON)
find_package(FLEX)

# generate c files for parser
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/parser)
BISON_TARGET(Parser lc3.y ${PROJECT_BINARY_DIR}/parser/parser.cpp)
FLEX_TARGET(Scanner lc3.l ${PROJECT_BINARY_DIR}/parser/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

set_source_files_properties(${PROJECT_BINARY_DIR}/parser/parser.cpp PROPERTIES GENERATED 1)
set_source_files_properties(${PROJECT_BINARY_DIR}/parser/lexer.cpp PROPERTIES GENERATED 1)
set_source_files_properties(${PROJECT_BINARY_DIR}/parser/parser.hpp PROPERTIES GENERATED 1)

# get all necessary files
file(GLOB CXX_AS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(AS_SRCS ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS} ${CXX_AS_FILES})

# find directories with includes
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR}/parser)  # need this because bison generates parser.hpp

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(lc3as ${AS_SRCS})
target_link_libraries(lc3as lc3utils)
target_link_libraries(lc3as jsonxx)
