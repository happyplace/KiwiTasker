cmake_minimum_required(VERSION 3.0)

project(KiwiTasker)

include_directories(include)

set(kiwi-tasker-src
    "include/kiwi/SpinLock.h"
    "src/Scheduler.cpp"
)

add_library(KiwiTasker STATIC ${kiwi-tasker-src})
