# create variable to identify resources
# set(ResPath ${PROJECT_SOURCE_DIR}/core/res)
# configure_file(paths.h ${PROJECT_SOURCE_DIR}/core/src/configured_paths.h)

# find packages
find_package(Threads)
include(GenerateExportHeader)

# get all necessary files
file(GLOB CXX_SOURCES ${PROJECT_SOURCE_DIR}/backend/src/*.cpp)
file(GLOB CXX_HEADERS ${PROJECT_SOURCE_DIR}/backend/src/*.h)

# generate library out of assembler
add_library(lc3core_obj OBJECT ${CXX_SOURCES} ${CXX_HEADERS})

add_library(lc3core_shared SHARED $<TARGET_OBJECTS:lc3core_obj>)
add_library(lc3core_static STATIC $<TARGET_OBJECTS:lc3core_obj>)
target_link_libraries(lc3core_shared ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(lc3core_static ${CMAKE_THREAD_LIBS_INIT})

generate_export_header(lc3core_shared
    BASE_NAME lc3core_shared
    EXPORT_MACRO_NAME CORE_EXPORT
    EXPORT_FILE_NAME lc3interface.h
    STATIC_DEFINE CORE_BUILT_AS_STATIC
)