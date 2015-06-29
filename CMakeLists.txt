cmake_minimum_required(VERSION 3.2)

# set project name
project(lc3sim)

# set defaults if no build type specified
#    - debug mode
#    - print level 9 (verbose)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug'")
    message(STATUS "Setting print level to 9")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Type of build" FORCE)
    set(PRINT_LEVEL 9 CACHE STRING "Print level" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# if no print type is specified or auto created, select 2, which shows info
if(NOT PRINT_LEVEL)
    message(STATUS "Setting print level to 2")
    set(PRINT_LEVEL 2 CACHE STRING "Print level" FORCE)
endif()

# simple error check on print level
if(NOT PRINT_LEVEL MATCHES "^[0-9]$")
    message(FATAL_ERROR "LOG_LEVEL must be a number from 0 to 9")
endif()

# set build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_PRINT_LEVEL=${PRINT_LEVEL}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -w")

# give option to build tests (download google test/google mock)
option(test "Build all tests." OFF)

if(test)
    include(ExternalProject)

    ExternalProject_Add(
        gtest
        URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        INSTALL_COMMAND ""
    )

    add_library(libgtest IMPORTED STATIC GLOBAL)
    add_dependencies(libgtest gtest)

    ExternalProject_Get_Property(gtest source_dir binary_dir)
    set_target_properties(libgtest PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    )

    include_directories("${source_dir}/include")

    #ExternalProject_Add(
        #gmock
        #URL https://googlemock.googlecode.com/files/gmock-1.7.0.zip
        #PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gmock
        #INSTALL_COMMAND ""
    #)

    #add_library(libgmock IMPORTED STATIC GLOBAL)
    #add_dependencies(libgmock gmock)

    #ExternalProject_Get_Property(gmock source_dir binary_dir)
    #set_target_properties(libgmock PROPERTIES
        #"IMPORTED_LOCATION" "${binary_dir}/libgmock.a"
        #"IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    #)

    #include_directories("${source_dir}/include")

    subdirs(test)
endif()

# recurse into sources
subdirs(src)
subdirs(thirdparty)
