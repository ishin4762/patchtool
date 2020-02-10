// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_LIB_PATCHFILE_PATCHFILEFACTORY_H_
#define PATCHTOOL_LIB_PATCHFILE_PATCHFILEFACTORY_H_

#include <string>
#include "PlainPatchFile.h"
#include "BZip2PatchFile.h"
#include "ZLibPatchFile.h"

class PatchFileFactory {
 public:
    /**
     * factory method.
     */
    static PatchFile* create(
        const std::string& compressMode,
        const std::string& executableOS) {
        if (compressMode == "zlib") {
            return new ZLibPatchFile(executableOS);
        } else if (compressMode == "bzip2") {
            return new BZip2PatchFile(executableOS);
        }
        return new PlainPatchFile(executableOS);
    }
};

#endif  // PATCHTOOL_LIB_PATCHFILE_PATCHFILEFACTORY_H_
