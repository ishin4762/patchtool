bin_PROGRAMS = patchgen

patchgen_SOURCES = patchgen.cc

patchgen_CXXFLAGS = -I../lib/patchfile -std=c++17

patchgen_LDADD = \
../lib/patchfile/libpatchfile.la \
../lib/bsdiff/libbsdiff.la \
../lib/bsdiff/libbspatch.la

