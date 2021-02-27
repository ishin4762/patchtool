cmake_minimum_required(VERSION 3.1)
project(patchtool_test CXX)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WINDOWS)
    add_definitions(-DWINDOWS)
endif()

include_directories(../src/)
link_directories(../src/lib)

add_subdirectory(lib)
