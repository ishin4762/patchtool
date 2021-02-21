cmake_minimum_required(VERSION 3.1)

add_executable(FileListTest FileListTest.cc)
target_link_libraries(
    FileListTest
    patchfile
    bsdiff
    bspatch
    bz2
    stdc++fs
    gtest
    gmock_main
)
add_test(NAME FileList COMMAND FileListTest)
