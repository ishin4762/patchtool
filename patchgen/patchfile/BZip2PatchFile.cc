// Copyright (C) 2020 ISHIN.
#include <cstring>
#include "BZip2PatchFile.h"

static int bz2_write(
    bsdiff_stream* stream,
    const void* buffer,
    int size) {
    int bz2err;
    BZFILE * bz2;

    bz2 = static_cast<BZFILE*>(stream->opaque);
    BZ2_bzWrite(&bz2err, bz2, const_cast<void*>(buffer), size);
    if (bz2err != BZ_STREAM_END && bz2err != BZ_OK) {
        return -1;
    }

    return 0;
}

BZip2PatchFile::BZip2PatchFile(
    const std::string& executableOS) :
    PatchFile(executableOS) {
    memset(&bz2, 0, sizeof(bz2));
    stream.malloc = malloc;
    stream.free = free;
    stream.write = bz2_write;
}

bool BZip2PatchFile::encode(
    const std::string& oldDir,
    const std::string& newDir,
    const std::string& output) {
    return true;
}


bool BZip2PatchFile::decode() {
    // Do nothing.
    return true;
}
