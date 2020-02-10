// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_LIB_PATCHFILE_PLAINPATCHFILE_H_
#define PATCHTOOL_LIB_PATCHFILE_PLAINPATCHFILE_H_

#include <bzlib.h>
#include <string>
#include "PatchFile.h"

class PlainPatchFile : public PatchFile {
 public:
    explicit PlainPatchFile(const std::string& executableOS);
    bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output);
    bool decode();

 private:
    FILE* file;
};

#endif  // PATCHTOOL_LIB_PATCHFILE_PLAINPATCHFILE_H_
