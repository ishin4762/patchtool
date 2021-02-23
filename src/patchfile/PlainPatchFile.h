// Copyright (C) 2020 ISHIN.
#ifndef SRC_PATCHFILE_PLAINPATCHFILE_H_
#define SRC_PATCHFILE_PLAINPATCHFILE_H_

#include <string>
#include "PatchFile.h"

class PlainPatchFile : public PatchFile {
 public:
    PlainPatchFile();

 protected:
    bool openWriter(FILE* fp);
    bool closeWriter();
    bool openReader(FILE* fp);
    bool closeReader();
};

#endif  // SRC_PATCHFILE_PLAINPATCHFILE_H_
