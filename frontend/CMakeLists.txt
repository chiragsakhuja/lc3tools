# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/backend/src)

find_package(Threads)

# get common files
file(GLOB COMMON_CXX_SOURCES ${PROJECT_SOURCE_DIR}/frontend/common/*.cpp)
file(GLOB COMMON_CXX_HEADERS ${PROJECT_SOURCE_DIR}/frontend/common/*.h)
# do not recompile for each target
add_library(frontend_common OBJECT ${COMMON_CXX_SOURCES} ${COMMON_CXX_HEADERS})

# recurse into sources
add_subdirectory(cli)
add_subdirectory(grader)
