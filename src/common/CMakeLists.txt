# find directories with includes
include_directories(../backend)

# get all necessary files
file(GLOB CXX_SOURCES *.cpp)
file(GLOB CXX_HEADERS *.h)

# generate package
add_library(common OBJECT ${CXX_SOURCES} ${CXX_HEADERS})
