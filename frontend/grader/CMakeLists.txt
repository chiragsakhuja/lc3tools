# find directories with includes
include_directories(${PROJECT_SOURCE_DIR}/backend/src ${PROJECT_SOURCE_DIR}/frontend/common)

find_package(Threads)

add_library(grader_common OBJECT framework.cpp framework.h)

file(GLOB LAB_SOURCES ${PROJECT_SOURCE_DIR}/frontend/grader/labs/*.cpp)

foreach(LAB_SOURCE ${LAB_SOURCES})
    get_filename_component(LAB_NAME ${LAB_SOURCE} NAME_WE)
    add_executable(${LAB_NAME} ${LAB_SOURCE} $<TARGET_OBJECTS:frontend_common> $<TARGET_OBJECTS:grader_common>)
    target_link_libraries(${LAB_NAME} lc3core_static ${CMAKE_THREAD_LIBS_INIT})
endforeach()

