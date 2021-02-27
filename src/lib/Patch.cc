// Copyright (C) 2020 ISHIN.
#include <cstring>
#include <iostream>
#include <sstream>

#include "include/patchtool.h"
#include "PatchFileFactory.h"
#include "PatchEncoder.h"
#include "PatchDecoder.h"
#include "FileAccess.h"

namespace patchtool {

bool Patch::encode(
    const std::string& oldDir,
    const std::string& newDir,
    const std::string& output,
    const std::string& mode,
    bool isHiddenSearch,
    const std::string& ignorePattern,
    uint32_t blockSize,
    bool isVerbose) {

    PatchFileFactory patchFileFactory;
    FileAccess fileAccess;
    std::unique_ptr<PatchFile> patchFile(
        patchFileFactory.create(mode));
    PatchEncoder patchEncoder(
        patchFile.get(), &fileAccess, isVerbose, blockSize);

    return patchEncoder.encode(
        oldDir, newDir, output,
        isHiddenSearch, ignorePattern);
}

bool Patch::decode(
    const std::string& targetDir,
    const std::string& input,
    bool isVerbose) {

    PatchFileFactory patchFileFactory;
    FileAccess fileAccess;
    std::unique_ptr<PatchFile> patchFile(
        patchFileFactory.fromFile(input));
    if (patchFile.get() == nullptr) {
        return false;
    }
    FILE *fp = fileAccess.openReadFile(input);
    PatchDecoder patchDecoder(
        patchFile.get(), &fileAccess,
        fp, 0, isVerbose);

    bool ret = patchDecoder.decode(targetDir);
    fileAccess.closeFile(fp);
    return ret;
}

bool Patch::decode(
    const std::string& targetDir,
    FILE *fp,
    const uint64_t offset,
    bool isVerbose) {

    PatchFileFactory patchFileFactory;
    FileAccess fileAccess;
    std::unique_ptr<PatchFile> patchFile(
        patchFileFactory.fromFilePointer(fp, offset));
    PatchDecoder patchDecoder(
        patchFile.get(), &fileAccess,
        fp, offset, isVerbose);

    return patchDecoder.decode(targetDir);
}

bool Patch::attach(
    const std::string& target,
    const std::string& out,
    const std::string& resource) {

    FileAccess fileAccess;
    const int PAYLOAD_ALIGNMENT = 8;
    FILE *inFp = nullptr;
    FILE *resFp = nullptr;
    FILE *outFp = nullptr;
    bool ret = true;

    try {
        // open base
        inFp = fileAccess.openReadFile(target);
        if (inFp == nullptr) {
            throw std::runtime_error("cannot open " + target);
        }
        // get in-size
        uint64_t inSize = 0;
        fileAccess.seek(inFp, 0, SEEK_END);
        inSize = fileAccess.tell(inFp);
        fileAccess.seek(inFp, 0, SEEK_SET);

        // open resource
        resFp = fileAccess.openReadFile(resource);
        if (resFp == nullptr) {
            throw std::runtime_error("cannot open " + resource);
        }

        // open out
        outFp = fileAccess.openWriteFile(out);
        if (outFp == nullptr) {
            throw std::runtime_error("cannot open " + out);
        }

        // copy input into output
        const int BUFFER_SIZE = 8192;
        char buffer[BUFFER_SIZE];
        fileAccess.seek(inFp, 0, SEEK_SET);
        int readSize = 0;
        do {
            readSize = fread(buffer, sizeof(char), BUFFER_SIZE, inFp);
            fwrite(buffer, sizeof(char), readSize, outFp);
        } while (readSize > 0);

        // add resource
        uint64_t resSize = 0;
        do {
            readSize = fread(buffer, sizeof(char), BUFFER_SIZE, resFp);
            resSize += readSize;
            fwrite(buffer, sizeof(char), readSize, outFp);
        } while (readSize > 0);

        // add padding
        const uint64_t paddingSize =
            PAYLOAD_ALIGNMENT - ((inSize+resSize) % PAYLOAD_ALIGNMENT);
        const char zeroBuf = '\0';
        for (int i=0; i < paddingSize; i++) {
            fwrite(&zeroBuf, sizeof(char), 1, outFp);
        }

        // put resource offest (8 bytes)
        fwrite(&inSize, sizeof(uint64_t), 1, outFp);

        // put footer (16 bytes)
        const char footer[16] = "END_OF_RESOURCE";
        fwrite(footer, 1, 16, outFp);
    } catch (const std::runtime_error& ex) {
        // error occurred.
        std::cerr << ex.what() << std::endl;
        ret = false;
    }

    // closing
    fileAccess.closeFile(inFp);
    fileAccess.closeFile(resFp);
    fileAccess.closeFile(outFp);

    return ret;
}

bool Patch::getResource(
    FILE* fp, uint64_t* offset) {

    FileAccess fileAccess;
    fileAccess.seek(fp, 0, SEEK_END);
    uint64_t fileSize = fileAccess.tell(fp);

    // validate footer data.
    if (fileSize < 24) {
        std::cerr << "invalid data." << std::endl;
        return false;
    }

    fileAccess.seek(fp, fileSize - 24, SEEK_SET);
    fread(offset, 8, 1, fp);

    char buf[16] = { 0 };
    const char footer[16] = "END_OF_RESOURCE";
    fread(buf, 1, 16, fp);
    if (memcmp(buf, footer, 16) != 0) {
        std::cerr << "invalid data." << std::endl;
        return false;
    }

    return true;
}

}  // namespace patchtool
