cmake_minimum_required(VERSION 3.1)
project(patchtool-cli CXX)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../)
link_directories(../patchfile ../lib/bsdiff ../lib/bzip2)

add_executable(patchgen patchgen.cc)
add_executable(patchapply patchapply.cc)
add_executable(selfapply selfapply.cc)

target_link_libraries(patchgen
    patchfile
    bsdiff
    bspatch
    bz2
    stdc++fs
)

target_link_libraries(patchapply
    patchfile
    bsdiff
    bspatch
    bz2
    stdc++fs
)

target_link_libraries(selfapply
    patchfile
    bsdiff
    bspatch
    bz2
    stdc++fs
)

if(WINDOWS)
    add_definitions(-DWINDOWS)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()
