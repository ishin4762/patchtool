bin_PROGRAMS = patchapply

patchapply_SOURCES = patchapply.cc

if WINDOWS
WINCXXFLAGS = -static-libgcc -Wl,-Bstatic -lpthread -DWINDOWS -fexec-charset=CP932
endif

patchapply_CXXFLAGS = -I../lib/patchfile -std=gnu++17 -lstdc++fs $(WINCXXFLAGS) -lbz2

patchapply_LDADD = \
../lib/patchfile/libpatchfile.la \
../lib/bsdiff/libbsdiff.la \
../lib/bsdiff/libbspatch.la
