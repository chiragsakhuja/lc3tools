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

file(GLOB SRC_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB AS_HEADERS ${PROJECT_SOURCE_DIR}/src/as/*.h)
file(GLOB UTILS_HEADERS ${PROJECT_SOURCE_DIR}/src/utils/*.h)
file(GLOB PARSER_HEADERS ${PROJECT_BINARY_DIR}/parser/*.hpp)

set(SRCS ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS} ${CXX_FILES} ${SRC_HEADERS} ${AS_HEADERS} ${UTILS_HEADERS} ${PARSER_HEADERS})

add_custom_target(parser DEPENDS ${CMAKE_BINARY_DIR}/parser/parser.hpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(lc3as ${SRCS})
add_dependencies(lc3as parser)
target_link_libraries(lc3as lc3utils jsonxx)

if(test)
    list(REMOVE_ITEM SRCS main.cpp)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/test)
    add_library(testlc3as STATIC ${SRCS})
    target_link_libraries(testlc3as lc3utils jsonxx)
    add_dependencies(testlc3as parser)
endif()
