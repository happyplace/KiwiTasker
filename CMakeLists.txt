cmake_minimum_required(VERSION 3.0)

project(KiwiTasker)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Werror")
endif()

include_directories(include)

set(kiwi-tasker-src
    "include/kiwi/SpinLock.h"
    "src/Scheduler.cpp"
)

add_library(KiwiTasker STATIC ${kiwi-tasker-src})
