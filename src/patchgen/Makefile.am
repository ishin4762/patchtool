bin_PROGRAMS = patchgen

patchgen_SOURCES = patchgen.cc

if WINDOWS
WINCXXFLAGS = -static-libgcc -Wl,-Bstatic -lpthread -DWINDOWS -fexec-charset=CP932
endif

patchgen_CXXFLAGS = -I../ -std=c++17 -lstdc++fs $(WINCXXFLAGS)

patchgen_LDADD = \
../patchfile/libpatchfile.la \
../lib/bsdiff/libbsdiff.la \
../lib/bsdiff/libbspatch.la \
../lib/bzip2/libbz2.a
