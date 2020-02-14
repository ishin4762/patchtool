// Copyright (C) 2020 ISHIN.
#ifndef LIB_PATCHFILE_BZIP2PATCHFILE_H_
#define LIB_PATCHFILE_BZIP2PATCHFILE_H_

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

#endif  // LIB_PATCHFILE_BZIP2PATCHFILE_H_
