cmake_minimum_required(VERSION 3.17)
project(bonk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -fsanitize=address)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.hpp)
add_executable(bonk ${SOURCES})
target_include_directories(bonk PUBLIC src)

add_subdirectory(test)
add_subdirectory(bonk_stdlib)