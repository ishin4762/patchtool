// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_PATCHGEN_PATCHFILE_ZLIBPATCHFILE_H_
#define PATCHTOOL_PATCHGEN_PATCHFILE_ZLIBPATCHFILE_H_

#include <string>
#include "PatchFile.h"

class ZLibPatchFile : public PatchFile {
 public:
    explicit ZLibPatchFile(const std::string& executableOS);
    bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output);
    bool decode();
};

#endif  // PATCHTOOL_PATCHGEN_PATCHFILE_ZLIBPATCHFILE_H_
