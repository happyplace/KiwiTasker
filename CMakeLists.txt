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

set (CMAKE_CXX_STANDARD 11)

include_directories(include)

include_directories(src)

set(kiwi-tasker-src

)

set(kiwi-tasker-src ${kiwi-tasker-src}

)

#find_package (Threads REQUIRED)

#add_library(KiwiTasker DYNAMIC ${kiwi-tasker-src})

#target_link_libraries(KiwiTasker Threads::Threads)
