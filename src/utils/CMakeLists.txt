# compile all cpp files
file(GLOB CXX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SRCS ${CXX_FILES})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_library(lc3utils STATIC ${SRCS})
