# create variable to identify resources
set(ResPath ${PROJECT_SOURCE_DIR}/res)
# copy file
configure_file(paths.h ${CMAKE_BINARY_DIR}/config/paths.h)
# include directory of paths
include_directories(${CMAKE_BINARY_DIR}/config)

subdirs(utils)
subdirs(as)
