cmake_minimum_required(VERSION 3.1)
project(patchtool CXX)

add_subdirectory(external_lib)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

link_directories(external_lib)

add_library(patchtool STATIC
    BZip2PatchFile.cc
    FileAccess.cc
    FileList.cc
    Patch.cc
    PatchDecoder.cc
    PatchEncoder.cc
    PatchFileFactory.cc
    PlainPatchFile.cc
)

target_link_libraries(patchtool
    bsdiff
    bspatch
    bz2
    stdc++fs
)

if(WINDOWS)
    add_definitions(-DWINDOWS)
endif()
