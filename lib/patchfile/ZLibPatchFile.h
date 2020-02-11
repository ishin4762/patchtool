// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_LIB_PATCHFILE_ZLIBPATCHFILE_H_
#define PATCHTOOL_LIB_PATCHFILE_ZLIBPATCHFILE_H_

#include <string>
#include "PatchFile.h"

class ZLibPatchFile : public PatchFile {
 public:
    ZLibPatchFile();
    bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output);
    bool decode(
        const std::string& targetDir,
        const std::string& input);

 protected:
    bool openWriter(FILE* fp);
    bool closeWriter();
    bool openReader(FILE* fp);
    bool closeReader();
};

#endif  // PATCHTOOL_LIB_PATCHFILE_ZLIBPATCHFILE_H_
