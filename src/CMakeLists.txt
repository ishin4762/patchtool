cmake_minimum_required(VERSION 3.1)
project(patchtool CXX)

set(CMAKE_CXX_FLAGS "-s -O3")
set(CMAKE_CC_FLAGS "-s -O3")

add_subdirectory(lib)
add_subdirectory(cli)
