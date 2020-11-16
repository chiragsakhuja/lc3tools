# find directories with includes
include_directories(../backend)
include_directories(../common)

# generate package
add_library(framework OBJECT framework.cpp framework.h)

# get all necessary files
if(BUILD_SAMPLES)
    file(GLOB TEST_SOURCES testers/*.cpp testers/samples/*.cpp)
else()
    file(GLOB TEST_SOURCES testers/*.cpp)
endif()

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE} $<TARGET_OBJECTS:common> $<TARGET_OBJECTS:framework>)
    target_include_directories(${TEST_NAME} PUBLIC .)
    target_link_libraries(${TEST_NAME} lc3core)
endforeach()

