bin_PROGRAMS = patchgen

patchgen_SOURCES = patchgen.cc

if WINDOWS
WINCXXFLAGS = -static-libgcc -Wl,-Bstatic -lpthread -DWINDOWS -fexec-charset=CP932
endif

patchgen_CXXFLAGS = -I../lib/patchfile -std=c++17 -Wl,-Bstatic -lbz2 $(WINCXXFLAGS)

patchgen_LDADD = \
../lib/patchfile/libpatchfile.la \
../lib/bsdiff/libbsdiff.la \
../lib/bsdiff/libbspatch.la
