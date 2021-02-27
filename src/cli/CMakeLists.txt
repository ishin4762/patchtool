cmake_minimum_required(VERSION 3.1)
project(patchtool-cli CXX)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../ .)
link_directories(../lib)

add_executable(patchgen patchgen.cc)
add_executable(patchapply patchapply.cc)
add_executable(selfapply selfapply.cc)

target_link_libraries(patchgen
    patchtool
    stdc++fs
)

target_link_libraries(patchapply
    patchtool
    stdc++fs
)

target_link_libraries(selfapply
    patchtool
    stdc++fs
)

if(WINDOWS)
    add_definitions(-DWINDOWS)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()
