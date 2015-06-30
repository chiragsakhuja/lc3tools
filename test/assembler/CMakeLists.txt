# grab all tests
file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# include assembler directory for ease
include_directories(${PROJECT_SOURCE_DIR}/src/as)

# generate test driver
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/test)
add_executable(test_assembler_simple ${SRCS})
target_link_libraries(test_assembler_simple lc3as libgtest)

# indicate to cmake that this is a test so it can be run with make test
add_test(test_assembler_simple ${PROJECT_BINARY_DIR}/bin/test/test_assembler_simple)
