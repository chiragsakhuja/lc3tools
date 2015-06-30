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
file(GLOB CXX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# find directories with includes
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR}/parser)  # need this because bison generates parser.hpp

# add headers as dependencies
file(GLOB SRC_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB AS_HEADERS ${PROJECT_SOURCE_DIR}/src/as/*.h)
file(GLOB UTILS_HEADERS ${PROJECT_SOURCE_DIR}/src/utils/*.h)
file(GLOB PARSER_HEADERS ${PROJECT_BINARY_DIR}/parser/*.hpp)

# grab all cpp files, except for driver (main.cpp)
set(SRCS ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS} ${CXX_FILES} ${SRC_HEADERS} ${AS_HEADERS} ${UTILS_HEADERS} ${PARSER_HEADERS})
list(REMOVE_ITEM SRCS main.cpp)

# create target so that library depends on parser having been generated
add_custom_target(parser DEPENDS ${CMAKE_BINARY_DIR}/parser/parser.hpp)

# generate library out of assembler
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_library(lc3as STATIC ${SRCS})
target_link_libraries(lc3as lc3utils jsonxx)

# generate driver, and have it link against assembler library
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(assembler main.cpp)
target_link_libraries(assembler lc3as)
