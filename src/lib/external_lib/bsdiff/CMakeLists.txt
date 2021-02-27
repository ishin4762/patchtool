cmake_minimum_required(VERSION 3.1)
project(bsdiff C)

add_library(bsdiff STATIC bsdiff.c)
add_library(bspatch STATIC bspatch.c)
