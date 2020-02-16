cmake_minimum_required(VERSION 3.1)
project(patchfile CXX)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../)

add_library(patchfile STATIC
    BZip2PatchFile.cc
    FileList.cc
    PatchFile.cc
    PlainPatchFile.cc
    ResourceAttacher.cc
)

if(WINDOWS)
    add_definitions(-DWINDOWS)
endif()
