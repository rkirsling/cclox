cmake_minimum_required(VERSION 3.10.2)
project(cclox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -O3 -flto")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
add_executable(cclox ${SOURCES})
