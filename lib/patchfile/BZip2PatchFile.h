// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_LIB_PATCHFILE_BZIP2PATCHFILE_H_
#define PATCHTOOL_LIB_PATCHFILE_BZIP2PATCHFILE_H_

#include <bzlib.h>
#include <string>
#include "PatchFile.h"

class BZip2PatchFile : public PatchFile {
 public:
    explicit BZip2PatchFile(const std::string& executableOS);
    bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output);
    bool decode();

 private:
    BZFILE* bz2;
};

#endif  // PATCHTOOL_LIB_PATCHFILE_BZIP2PATCHFILE_H_
