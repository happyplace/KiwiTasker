cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0079 NEW)

project(kiwi-example C)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Werror")
endif()

#include_directories("../include")
#add_subdirectory(.. build EXCLUDE_FROM_ALL)

add_executable(kiwi-example main.c)

#target_link_libraries(kiwi-example KiwiTasker)
