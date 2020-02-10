// Copyright (C) 2020 ISHIN.
#include <iostream>
#include <cstring>

#include "PlainPatchFile.h"
#include "FileList.h"

static int plain_write(
    bsdiff_stream* stream,
    const void* buffer,
    int size) {
    size_t ret;
    FILE* file;

    file = static_cast<FILE*>(stream->opaque);
    ret = fwrite(const_cast<void*>(buffer), 1, size, file);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

/**
 * constructor.
 */
PlainPatchFile::PlainPatchFile(
    const std::string& executableOS) :
    PatchFile(executableOS) {
    memset(&file, 0, sizeof(file));
    stream.malloc = malloc;
    stream.free = free;
    stream.write = plain_write;
}

bool PlainPatchFile::encode(
    const std::string& oldDir,
    const std::string& newDir,
    const std::string& output) {

    // search directory diff.
    FileList diffList = searchDiff(oldDir, newDir);

    // write file info.
    file = fopen(output.c_str(), "wb");
    if (file == nullptr) {
        std::cerr << "cannot create " << output << std::endl;
        return false;
    }
    writeFileInfo(file, diffList);

    // flush.
    fclose(file);

    return true;
}


bool PlainPatchFile::decode() {
    // Do nothing.
    return true;
}
