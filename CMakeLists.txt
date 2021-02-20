cmake_minimum_required(VERSION 3.0)

project(KiwiTasker)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Werror")
endif()

include_directories(include)

include_directories(src)

set(kiwi-tasker-src
    "include/kiwi/Scheduler.h"
    "include/kiwi/SpinLock.h"
    "include/kiwi/Context.h"
    "include/kiwi/Counter.h"
    "include/kiwi/Fiber.h"
    "include/kiwi/FiberWorkerStorage.h"
    "include/kiwi/Job.h"
    "include/kiwi/JobPriority.h"
    "include/kiwi/Config.h"
    "include/kiwi/Queue.h"
    "include/kiwi/FiberPool.h"
)

set(kiwi-tasker-src ${kiwi-tasker-src}
    "src/Scheduler.cpp"    
    "src/Counter.cpp"
    "src/FiberPool.cpp"
    "src/FiberWorkerStorage.cpp"
)

set(kiwi-tasker-src ${kiwi-tasker-src}
    "src/SchedulerImpl_unix.h"
    "src/SchedulerImpl_unix.cpp"
)

find_package (Threads REQUIRED)

add_library(KiwiTasker STATIC ${kiwi-tasker-src})

target_link_libraries(KiwiTasker Threads::Threads)
