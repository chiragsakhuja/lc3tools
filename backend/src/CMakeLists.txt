# create variable to identify resources
# set(ResPath ${PROJECT_SOURCE_DIR}/core/res)
# configure_file(paths.h ${PROJECT_SOURCE_DIR}/core/src/configured_paths.h)

# find packages
find_package(BISON)
find_package(FLEX)
find_package(Threads)

set(PARSER_DIR ${PROJECT_SOURCE_DIR}/backend/src/parser_gen)

# generate c files for parser
file(MAKE_DIRECTORY ${PARSER_DIR})
BISON_TARGET(Parser lc3.y ${PARSER_DIR}/parser.cpp)
FLEX_TARGET(Scanner lc3.l ${PARSER_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

# mark files as generated
set_source_files_properties(${PARSER_DIR}/parser.cpp PROPERTIES GENERATED 1 COMPILE_FLAGS -w)
set_source_files_properties(${PARSER_DIR}/lexer.cpp PROPERTIES GENERATED 1 COMPILE_FLAGS -w)
set_source_files_properties(${PARSER_DIR}/parser.hpp PROPERTIES GENERATED 1 COMPILE_FLAGS -w)
# set_source_files_properties(${PROJECT_BINARY_DIR}/configured_paths.h PROPERTIES GENERATED 1)

include_directories(${PARSER_DIR})  # need this because bison generates parser.hpp

# get all necessary files
file(GLOB CXX_SOURCES ${PROJECT_SOURCE_DIR}/backend/src/*.cpp)
file(GLOB CXX_HEADERS ${PROJECT_SOURCE_DIR}/backend/src/*.h)

add_custom_target(parser DEPENDS ${PARSER_DIR}/parser.cpp)

# generate library out of assembler
add_library(lc3core STATIC ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS} ${CXX_SOURCES} ${CXX_HEADERS})
target_link_libraries(lc3core ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(lc3core parser)
