# include assembler directory for ease
include_directories(${PROJECT_SOURCE_DIR}/src/as)
include_directories(${PROJECT_BINARY_DIR}/parser)

### NEED TO COPY THE FOLLOWING SECTION FOR EVERY TEST
# generate test driver
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/test)
add_executable(test_assembler_simple assembler_simple.cpp)
target_link_libraries(test_assembler_simple lc3as libgtest)

# indicate to cmake that this is a test so it can be run with make test
add_test(test_assembler_simple ${PROJECT_BINARY_DIR}/bin/test/test_assembler_simple)
### END SECTION
