// Copyright (C) 2020 ISHIN.
#ifndef SRC_PATCHFILE_BZIP2PATCHFILE_H_
#define SRC_PATCHFILE_BZIP2PATCHFILE_H_

#include <string>
extern "C" {
#include "lib/bzip2/bzlib.h"
}
#include "PatchFile.h"

class BZip2PatchFile : public PatchFile {
 public:
    BZip2PatchFile();
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

#endif  // SRC_PATCHFILE_BZIP2PATCHFILE_H_
