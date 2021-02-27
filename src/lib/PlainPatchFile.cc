// Copyright (C) 2020 ISHIN.
#include <iostream>
#include <cstring>

#include "PlainPatchFile.h"

static int plain_write(
    bsdiff_stream* stream,
    const void* buffer,
    int size) {
    int ret;
    FILE* file;

    file = static_cast<FILE*>(stream->opaque);
    ret = fwrite(const_cast<void*>(buffer), 1, size, file);
    if (ret <= 0) {
        return -1;
    }
    return 0;
}

static int plain_read(
    const bspatch_stream* stream,
    void* buffer,
    int size) {
    int ret;
    FILE* file;

    file = static_cast<FILE*>(stream->opaque);
    ret = fread(buffer, 1, size, file);
    if (ret != size) {
        return -1;
    }
    return 0;
}

namespace patchtool {

PlainPatchFile::PlainPatchFile() {
    writeStream.malloc = malloc;
    writeStream.free = free;
    writeStream.write = plain_write;
    readStream.read = plain_read;
}

bool PlainPatchFile::openWriter(FILE* fp) {
    writeStream.opaque = fp;
    return true;
}

bool PlainPatchFile::closeWriter() {
    // do nothing.
    return true;
}

bool PlainPatchFile::openReader(FILE* fp) {
    readStream.opaque = fp;
    return true;
}

bool PlainPatchFile::closeReader() {
    // do nothing.
    return true;
}

}  // namespace patchtool
