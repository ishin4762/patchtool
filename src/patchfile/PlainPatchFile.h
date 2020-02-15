// Copyright (C) 2020 ISHIN.
#ifndef SRC_PATCHFILE_PLAINPATCHFILE_H_
#define SRC_PATCHFILE_PLAINPATCHFILE_H_

#include <string>
#include "PatchFile.h"

class PlainPatchFile : public PatchFile {
 public:
    PlainPatchFile();
    bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output,
        bool isHiddenSearch,
        const std::string& ignorePattern);
    bool decode(
        const std::string& targetDir,
        const std::string& input);

 protected:
    bool openWriter(FILE* fp);
    bool closeWriter();
    bool openReader(FILE* fp);
    bool closeReader();
};

#endif  // SRC_PATCHFILE_PLAINPATCHFILE_H_
