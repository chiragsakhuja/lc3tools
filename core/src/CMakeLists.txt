# create variable to identify resources
set(ResPath ${PROJECT_SOURCE_DIR}/core/res)
configure_file(common/paths.h ${CMAKE_BINARY_DIR}/header_gen/paths_gen.h)
include_directories(${CMAKE_BINARY_DIR}/header_gen)

# used findFLEX
find_package(BISON)
find_package(FLEX)

# generate c files for parser
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/parser_gen)
BISON_TARGET(Parser asm/lc3.y ${PROJECT_BINARY_DIR}/parser_gen/parser.cpp)
FLEX_TARGET(Scanner asm/lc3.l ${PROJECT_BINARY_DIR}/parser_gen/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

set_source_files_properties(${PROJECT_BINARY_DIR}/parser_gen/parser.cpp PROPERTIES GENERATED 1)
set_source_files_properties(${PROJECT_BINARY_DIR}/parser_gen/lexer.cpp PROPERTIES GENERATED 1)
set_source_files_properties(${PROJECT_BINARY_DIR}/parser_gen/parser.hpp PROPERTIES GENERATED 1)

# get all necessary files
file(GLOB CXX_FILES ${PROJECT_SOURCE_DIR}/core/src/common/*.cpp ${PROJECT_SOURCE_DIR}/core/src/asm/*.cpp ${PROJECT_SOURCE_DIR}/core/src/sim/*.cpp)

# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/core/src)
include_directories(${PROJECT_BINARY_DIR}/parser_gen)  # need this because bison generates parser.hpp

# add headers as dependencies
file(GLOB CXX_HEADERS ${PROJECT_SOURCE_DIR}/core/src/common/*.h ${PROJECT_SOURCE_DIR}/core/src/asm/*.h ${PROJECT_SOURCE_DIR}/core/src/sim/*.h)
file(GLOB PARSER_HEADERS ${PROJECT_BINARY_DIR}/parser_gen/*.hpp)

# grab all cpp files, except for driver (main.cpp)
set(SRCS ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS} ${CXX_FILES} ${CXX_HEADERS} ${PARSER_HEADERS})

# create target so that library depends on parser having been generated
add_custom_target(parser DEPENDS ${CMAKE_BINARY_DIR}/parser_gen/parser.hpp)

# generate library out of assembler
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_library(lc3core SHARED ${SRCS})
target_link_libraries(lc3core jsonxx)
add_dependencies(lc3core parser)

# generate driver, and have it link against assembler library
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# add_executable(assembler main.cpp)
# target_link_libraries(assembler lc3as)

subdirs(thirdparty)
