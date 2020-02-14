lib_LTLIBRARIES = libpatchfile.la

libpatchfile_la_SOURCES = \
BZip2PatchFile.cc \
PlainPatchFile.cc \
PatchFile.cc \
FileList.cc

if WINDOWS
WINCXXFLAGS = -static-libgcc -Wl,-Bstatic -lpthread -DWINDOWS -fexec-charset=CP932
endif

libpatchfile_la_CXXFLAGS = -std=c++17 -lstdc++fs $(WINCXXFLAGS)

libpatchfile_la_LIBADD = \
../bzip2/libbz2.a
