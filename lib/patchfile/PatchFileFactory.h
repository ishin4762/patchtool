// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_LIB_PATCHFILE_PATCHFILEFACTORY_H_
#define PATCHTOOL_LIB_PATCHFILE_PATCHFILEFACTORY_H_

#include <string>
#include "PlainPatchFile.h"
#include "BZip2PatchFile.h"

class PatchFileFactory {
 public:
    /**
     * factory method.
     */
    static PatchFile* create(
        const std::string& compressMode) {
        if (compressMode == "bzip2") {
            return new BZip2PatchFile();
        }
        return new PlainPatchFile();
    }

    /**
     * factory method from file.
     */
    static PatchFile* fromFile(const std::string& file) {
        FILE *fp = fopen(file.c_str(), "rb");
        if (fp == nullptr) {
            return nullptr;
        }

        char signature[16];
        fread(signature, sizeof(char), sizeof(signature), fp);
        fclose(fp);
        std::string strSignature = signature;

        if (strSignature.substr(0, 5) == "PLAIN") {
            return new PlainPatchFile();
        } else if (strSignature.substr(0, 5) == "BZIP2") {
            return new BZip2PatchFile();
        }

        return nullptr;
    }
};

#endif  // PATCHTOOL_LIB_PATCHFILE_PATCHFILEFACTORY_H_
