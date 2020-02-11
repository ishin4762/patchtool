bin_PROGRAMS = patchapply

patchapply_SOURCES = patchapply.cc

patchapply_CXXFLAGS = -I../lib/patchfile -std=c++17

patchapply_LDADD = \
../lib/patchfile/libpatchfile.la \
../lib/bsdiff/libbsdiff.la \
../lib/bsdiff/libbspatch.la
