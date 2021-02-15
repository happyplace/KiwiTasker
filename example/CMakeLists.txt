cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0079 NEW)

project(kiwi-example)

include_directories(../include)
add_subdirectory(.. build EXCLUDE_FROM_ALL)

set(example-src
    "src/Scheduler.cpp"
)

add_executable(kiwi-example main.cpp)

target_link_libraries(kiwi-example KiwiTasker)
