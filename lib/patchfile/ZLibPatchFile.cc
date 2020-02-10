// Copyright (C) 2020 ISHIN.
#include <cstring>
#include "ZLibPatchFile.h"

ZLibPatchFile::ZLibPatchFile(
    const std::string& executableOS) :
    PatchFile(executableOS) {
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

bool ZLibPatchFile::decode() {
    // not implement.
    return true;
}
