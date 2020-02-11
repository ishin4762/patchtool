// Copyright (C) 2020 ISHIN.
#include <cstring>
#include <iostream>

#include "BZip2PatchFile.h"

static int bz2_write(
    bsdiff_stream* stream,
    const void* buffer,
    int size) {
    int bz2err;
    BZFILE* bz2;

    bz2 = static_cast<BZFILE*>(stream->opaque);
    BZ2_bzWrite(&bz2err, bz2, const_cast<void*>(buffer), size);
    if (bz2err != BZ_STREAM_END && bz2err != BZ_OK) {
        return -1;
    }

    return 0;
}

static int bz2_read(
    const bspatch_stream* stream,
    void* buffer,
    int size) {
    int bz2err;
    BZFILE* bz2;
    int n;

    bz2 = static_cast<BZFILE*>(stream->opaque);
    n = BZ2_bzRead(&bz2err, bz2, buffer, size);
    if (n != size) {
        return -1;
    }

    return 0;
}

BZip2PatchFile::BZip2PatchFile() {
    writeStream.malloc = malloc;
    writeStream.free = free;
    writeStream.write = bz2_write;
    readStream.read = bz2_read;

    // copy signature.
    const char SIGNATURE[16] = "BZIP2 ver.1.00";
    memcpy(signature, SIGNATURE, sizeof(signature));
}

bool BZip2PatchFile::encode(
    const std::string& oldDir,
    const std::string& newDir,
    const std::string& output) {

    // search directory diff.
    FileList diffList = searchDiff(oldDir, newDir);

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


bool BZip2PatchFile::decode(
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

    return true;
}

bool BZip2PatchFile::openWriter(FILE* fp) {
    // initialize bzWriter
    int bz2err = 0;
    BZFILE* bz2 = BZ2_bzWriteOpen(&bz2err, fp, 9, 0, 0);
    if (bz2 == nullptr || bz2err != BZ_OK) {
        std::cerr << "cannot initialize bzip2 writer. errCode="
            << bz2err << std::endl;
        return false;
    }

    writeStream.opaque = bz2;
    return true;
}

bool BZip2PatchFile::closeWriter() {
    // close bzWriter
    int bz2err;
    BZFILE* bz2 = static_cast<BZFILE*>(writeStream.opaque);
    BZ2_bzWriteClose(&bz2err, bz2, 0, NULL, NULL);
    if (bz2err != BZ_OK) {
        std::cerr << "cannot close bzip2 writer successfully. errCode="
            << bz2err << std::endl;
    }
    return true;
}

bool BZip2PatchFile::openReader(FILE* fp) {
    // initialize bzReader
    int bz2err;
    BZFILE* bz2 = BZ2_bzReadOpen(&bz2err, fp, 0, 0, NULL, 0);
    if (bz2 == nullptr || bz2err != BZ_OK) {
        std::cerr << "cannot initialize bzip2 reader. errCode="
            << bz2err << std::endl;
        return false;
    }

    readStream.opaque = bz2;
    return true;
}

bool BZip2PatchFile::closeReader() {
    // close bzReader
    int bz2err;
    BZFILE* bz2 = static_cast<BZFILE*>(readStream.opaque);
    BZ2_bzReadClose(&bz2err, bz2);
    return true;
}
