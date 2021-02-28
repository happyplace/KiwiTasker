cmake_minimum_required(VERSION 3.0)

project(KiwiTasker)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Werror")
endif()

if (MSVC)
    set(MSVC_COMMON_FLAGS 
	    /nologo     # Don't show version info
	    /W4         # Enable warning level 4
	    /MP         # Build with multiple processes
	    /WX         # Treat warnings as errors
    )
    string(REPLACE ";" " " MSVC_COMMON_FLAGS "${MSVC_COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS "${MSVC_COMMON_FLAGS}" CACHE STRING "C++ Common Flags" FORCE)
endif()

include_directories(include)

include_directories(src)

set(kiwi-tasker-src
    "include/kiwi/Array.h"
    "include/kiwi/Config.h"
    "include/kiwi/Context.h"
    "include/kiwi/ContextFunctions.h"
    "include/kiwi/Counter.h"
    "include/kiwi/Fiber.h"
    "include/kiwi/FiberPool.h"
    "include/kiwi/FiberWorkerStorage.h"
    "include/kiwi/Job.h"
    "include/kiwi/JobPriority.h"
    "include/kiwi/Queue.h"
    "include/kiwi/Scheduler.h"
    "include/kiwi/SchedulerImpl.h"
    "include/kiwi/SchedulerThreadImpl.h"
    "include/kiwi/SpinLock.h"
)

set(kiwi-tasker-src ${kiwi-tasker-src}
    "src/Counter.cpp"    
    "src/FiberPool.cpp"
    "src/FiberWorkerStorage.cpp"
    "src/Scheduler.cpp"    
    "src/SchedulerImpl.cpp"
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DKIWI_SCHEDULER_USE_PTHREAD)
    set(kiwi-tasker-src ${kiwi-tasker-src}
        "include/kiwi/ContextFunctionsLinux.h"
        "include/kiwi/SchedulerThreadImplPthreads.h"
        "src/SchedulerThreadImplPthreads.cpp"  
    )
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-DKIWI_SCHEDULER_USE_WINTHREADS)
    set(kiwi-tasker-src ${kiwi-tasker-src}
        "include/kiwi/ContextFunctionsWindows.h"
        "include/kiwi/SchedulerThreadImplWinThreads.h"
        "src/SchedulerThreadImplWinThreads.cpp"
    )
endif()

find_package (Threads REQUIRED)

add_library(KiwiTasker STATIC ${kiwi-tasker-src})

target_link_libraries(KiwiTasker Threads::Threads)
