set(ResPath ${PROJECT_SOURCE_DIR}/res)
configure_file(paths.h ${CMAKE_BINARY_DIR}/config/paths.h)
include_directories(${CMAKE_BINARY_DIR}/config)

subdirs(utils)
subdirs(as)
