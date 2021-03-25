cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0054 NEW)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


project(kiwi-example)

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

#include_directories("../external/deboost.context/include")
#add_subdirectory("../external/deboost.context" build EXCLUDE_FROM_ALL)
include_directories("../include")
add_subdirectory(.. build EXCLUDE_FROM_ALL)

add_executable(kiwi-example main.cpp) #"../external/simple-stdatomic-for-VS-Clang/stdatomic.h" "../external/simple-stdatomic-for-VS-Clang/stdatomic.c")

#set_property(TARGET kiwi-example PROPERTY C_STANDARD 11)
#set_property(TARGET kiwi-example PROPERTY C_STANDARD_REQUIRED ON)

#target_link_libraries(kiwi-example KiwiTasker)

target_link_libraries(kiwi-example d3dcompiler.lib)
target_link_libraries(kiwi-example D3D12.lib)
target_link_libraries(kiwi-example dxgi.lib)

target_link_libraries(kiwi-example KiwiTasker)
