// Copyright (C) 2020 ISHIN.
#include "ZLibPatchFile.h"

ZLibPatchFile::ZLibPatchFile(
    const std::string& executableOS) :
    PatchFile(executableOS) {
}

bool ZLibPatchFile::encode(
    const std::string& oldDir,
    const std::string& newDir,
    const std::string& output) {
    // Do nothing.
    return true;
}


bool ZLibPatchFile::decode() {
    // Do nothing.
    return true;
}
