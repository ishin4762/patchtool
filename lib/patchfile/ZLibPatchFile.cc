// Copyright (C) 2020 ISHIN.
#include <cstring>
#include "ZLibPatchFile.h"

ZLibPatchFile::ZLibPatchFile() {
    // copy signature.
    const char SIGNATURE[16] = "ZLIB ver.1.00";
    memcpy(signature, SIGNATURE, sizeof(signature));
}

bool ZLibPatchFile::encode(
    const std::string& oldDir,
    const std::string& newDir,
    const std::string& output) {
    // not implement.
    return true;
}

bool ZLibPatchFile::decode(
    const std::string& targetDir,
    const std::string& input) {
    // not implement.
    return true;
}

bool ZLibPatchFile::openWriter(FILE* fp) {
    // not implement.
    return true;
}

bool ZLibPatchFile::closeWriter() {
    // not implement.
    return true;
}

bool ZLibPatchFile::openReader(FILE* fp) {
    // not implement.
    return true;
}

bool ZLibPatchFile::closeReader() {
    // not implement.
    return true;
}
