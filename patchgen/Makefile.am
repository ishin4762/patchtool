bin_PROGRAMS = patchgen

patchgen_SOURCES = patchgen.cc

patchgen_CXXFLAGS = -Ipatchfile -std=c++17

patchgen_LDADD = patchfile/libpatchfile.la ../bsdiff/libbsdiff.la

SUBDIRS = patchfile

