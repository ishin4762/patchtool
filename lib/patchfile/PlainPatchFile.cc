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

/**
 * constructor.
 */
PlainPatchFile::PlainPatchFile() {
    writeStream.malloc = malloc;
    writeStream.free = free;
    writeStream.write = plain_write;
    readStream.read = plain_read;

    // copy signature.
    const char SIGNATURE[16] = "PLAIN ver.1.00";
    memcpy(signature, SIGNATURE, sizeof(signature));
}

bool PlainPatchFile::encode(
    const std::string& oldDir,
    const std::string& newDir,
    const std::string& output,
    bool isHiddenSearch,
    const std::string& ignorePattern) {

    // search directory diff.
    FileList diffList = searchDiff(
        oldDir, newDir, isHiddenSearch, ignorePattern);

    // write file.
    FILE* file = fopen(output.c_str(), "wb");
    if (file == nullptr) {
        std::cerr << "cannot create " << output << std::endl;
        return false;
    }
    create(file, &diffList);

    // flush.
    fclose(file);

    return true;
}


bool PlainPatchFile::decode(
    const std::string& targetDir,
    const std::string& input) {

    // read file.
    FILE* file = fopen(input.c_str(), "rb");
    if (file == nullptr) {
        std::cerr << "cannot open " << input << std::endl;
        return false;
    }
    bool ret = apply(targetDir, file);

    // finish.
    fclose(file);

    return ret;
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
