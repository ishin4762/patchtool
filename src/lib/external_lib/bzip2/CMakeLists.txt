# Copyright (c) 2018 Sergiu Deitsch
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTMETISLAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required (VERSION 3.1)
project (bzip2 C)

set (MAJOR_VERSION 1)
set (MINOR_VERSION 0)
set (VERSION_PATCH 6)
set (VERSION 1.0.6)

set (CPACK_COMPONENTS_ALL Runtime)
set (CPACK_COMPONENT_APPLICATIONS_GROUP Runtime)
set (CPACK_COMPONENT_LIBRARIES_GROUP Development)
set (CPACK_COMPONENT_HEADERS_GROUP Development)
set (CPACK_PACKAGE_NAME bzip2)
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "A high-quality data compression program")
set (CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION})
set (CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_PACKAGE_CONTACT "julian@bzip.org")

include (CMakePackageConfigHelpers)
include (CPack)
include (CTest)
include (GNUInstallDirs)

set (CMAKE_DEBUG_POSTFIX d)

add_library (bz2
  blocksort.c
  bzlib.c
  bzlib.h
  bzlib_private.h
  compress.c
  crctable.c
  decompress.c
  huffman.c
  randtable.c
)

if (BUILD_SHARED_LIBS)
  if (WIN32)
    target_sources (libz2 PRIVATE libbz2.def)
  endif (WIN32)
endif (BUILD_SHARED_LIBS)
