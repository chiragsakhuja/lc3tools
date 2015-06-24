cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug'")
    message(STATUS "Setting print level to 9")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Type of build" FORCE)
    set(PRINT_LEVEL 9 CACHE STRING "Print level" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

if(NOT PRINT_LEVEL)
    message(STATUS "Setting print level to 2")
    set(PRINT_LEVEL 2 CACHE STRING "Print level" FORCE)
endif()

if(NOT PRINT_LEVEL MATCHES "^[0-9]$")
    message(FATAL_ERROR "LOG_LEVEL must be a number from 0 to 9")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_PRINT_LEVEL=${PRINT_LEVEL}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -w")

project(lc3sim)
subdirs(src)
subdirs(thirdparty)
