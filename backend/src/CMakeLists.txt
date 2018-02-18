# create variable to identify resources
# set(ResPath ${PROJECT_SOURCE_DIR}/core/res)
# configure_file(paths.h ${PROJECT_SOURCE_DIR}/core/src/configured_paths.h)

# find packages
find_package(Threads)
include(GenerateExportHeader)

# get all necessary files
file(GLOB CXX_SOURCES ${PROJECT_SOURCE_DIR}/backend/src/*.cpp)
file(GLOB CXX_HEADERS ${PROJECT_SOURCE_DIR}/backend/src/*.h)

# generate library
add_library(lc3core SHARED ${CXX_SOURCES} ${CXX_HEADERS})
target_link_libraries(lc3core ${CMAKE_THREAD_LIBS_INIT})

#generate_export_header(lc3core
    #BASE_NAME lc3core
    #EXPORT_MACRO_NAME CORE_EXPORT
    #EXPORT_FILE_NAME lc3interface.h
    #STATIC_DEFINE CORE_BUILT_AS_STATIC
#)