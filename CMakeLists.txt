cmake_minimum_required(VERSION 3.1)
project(patchtool_root)

# compile
add_subdirectory(src)

# test
include(cmake/DownloadProject/DownloadProject.cmake)
download_project(
    PROJ googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG master
    UPDATE_DISCONNECTED 1
)
add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
enable_testing()

add_subdirectory(test)
