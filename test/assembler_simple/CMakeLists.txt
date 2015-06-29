file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

include_directories(${PROJECT_SOURCE_DIR}/src/as)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/test)
add_executable(test_assembler_simple ${SRCS})
target_link_libraries(test_assembler_simple testlc3as ${GTEST_LIBRARIES})
