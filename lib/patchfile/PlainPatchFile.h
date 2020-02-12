// Copyright (C) 2020 ISHIN.
#ifndef LIB_PATCHFILE_PLAINPATCHFILE_H_
#define LIB_PATCHFILE_PLAINPATCHFILE_H_

#include <bzlib.h>
#include <string>
#include "PatchFile.h"

class PlainPatchFile : public PatchFile {
 public:
    PlainPatchFile();
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

#endif  // LIB_PATCHFILE_PLAINPATCHFILE_H_
