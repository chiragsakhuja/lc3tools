# get all necessary files
file(GLOB CXX_SOURCES *.cpp)
file(GLOB CXX_HEADERS *.h)

# generate library
add_library(lc3core STATIC ${CXX_SOURCES} ${CXX_HEADERS})
target_link_libraries(lc3core)