cmake_minimum_required(VERSION 3.17)
project(bonk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -fsanitize=address)

file(GLOB_RECURSE SOURCES src/bonk/*.cpp src/bonk/*.hpp src/utils/*.cpp src/utils/*.hpp)

add_executable(bonk ${SOURCES} src/main.cpp)
target_include_directories(bonk PUBLIC src)

add_executable(bonk-metafile-viewer ${SOURCES} src/metafile_viewer.cpp)
target_include_directories(bonk-metafile-viewer PUBLIC src)

add_subdirectory(test)
add_subdirectory(bonk_stdlib)