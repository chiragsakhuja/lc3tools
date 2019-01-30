cmake_minimum_required(VERSION 2.8)

# set project name
project(lc3tools)

# if no build mode is specified, pick release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release'")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# give option to build tests (download google test/google mock)
# option(test "Build all tests." OFF)

# set build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_ENABLE_DEBUG")
if(NOT DEFINED MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra -Wno-format-security")
endif()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -w")
if(DEFINED MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -MT -EHsc")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

cmake_policy(SET CMP0042 NEW)

#if(test)
    #set(TestPath ${PROJECT_SOURCE_DIR}/test)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ENABLE_TESTING")

    #find_package(Threads REQUIRED)
    #enable_testing()

    #include(ExternalProject)

    #ExternalProject_Add(
        #gtest
        #URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
        #PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        #INSTALL_COMMAND ""
    #)

    #add_library(libgtest IMPORTED STATIC GLOBAL)
    #add_dependencies(libgtest gtest)

    #ExternalProject_Get_Property(gtest source_dir binary_dir)
    #set_target_properties(libgtest PROPERTIES
        #"IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
        #"IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    #)

    #include_directories("${source_dir}/include")

    #subdirs(test)
#endif()

add_subdirectory(backend)
add_subdirectory(frontend)
