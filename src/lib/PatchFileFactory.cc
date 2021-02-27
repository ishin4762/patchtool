// Copyright (C) 2020 ISHIN.
#include "PatchFileFactory.h"
#include "PlainPatchFile.h"
#include "BZip2PatchFile.h"

namespace patchtool {

PatchFile* PatchFileFactory::create(
    const std::string& compressMode) {

    if (compressMode == "bzip2") {
        return new BZip2PatchFile();
    }
    return new PlainPatchFile();
}

PatchFile* PatchFileFactory::fromFile(
    const std::string& file) {

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

PatchFile* PatchFileFactory::fromFilePointer(
    FILE* fp, uint64_t offset) {

    char signature[16];
    fseeko(fp, offset, SEEK_SET);
    fread(signature, sizeof(char), sizeof(signature), fp);
    std::string strSignature = signature;

    if (strSignature.substr(0, 5) == "PLAIN") {
        return new PlainPatchFile();
    } else if (strSignature.substr(0, 5) == "BZIP2") {
        return new BZip2PatchFile();
    }

    return nullptr;
}

}  // namespace patchtool
