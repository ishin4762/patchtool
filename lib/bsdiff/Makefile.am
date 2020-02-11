lib_LTLIBRARIES = libbsdiff.la libbspatch.la

libbsdiff_la_SOURCES = bsdiff.c bsdiff.h

libbspatch_la_SOURCES = bspatch.c bspatch.h

if WINDOWS
WINCXXFLAGS = -static-libgcc -Wl,-Bstatic -lpthread -DWINDOWS -fexec-charset=CP932
endif

libbsdiff_la_CFLAGS = $(WINCXXFLAGS)
libbspatch_la_CFLAGS = $(WINCXXFLAGS)
