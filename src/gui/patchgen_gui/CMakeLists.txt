cmake_minimum_required(VERSION 3.5)

project(patchgen_gui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES patchgen_gui_ja_JP.ts)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(patchgen_gui
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  ${QM_FILES}
)

target_link_libraries(patchgen_gui Qt5::Widgets)