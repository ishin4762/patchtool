// Copyright (C) 2020 ISHIN.
#include <cstring>
#include <iostream>
#include <sstream>
#include "ResourceAttacher.h"

bool ResourceAttacher::attach(
    const std::string& target,
    const std::string& out,
    const std::string& resource) {

    const int PAYLOAD_ALIGNMENT = 8;
    FILE *inFp = nullptr;
    FILE *resFp = nullptr;
    FILE *outFp = nullptr;
    bool ret = true;

    try {
        // open base
        inFp = fopen(target.c_str(), "rb");
        if (inFp == nullptr) {
            throw std::runtime_error("cannot open " + target);
        }
        // get in-size
        uint64_t inSize = 0;
        fseeko(inFp, 0, SEEK_END);
        inSize = ftello(inFp);
        fseeko(inFp, 0, SEEK_SET);

        // open resource
        resFp = fopen(resource.c_str(), "rb");
        if (resFp == nullptr) {
            throw std::runtime_error("cannot open " + resource);
        }

        // open out
        outFp = fopen(out.c_str(), "wb");
        if (outFp == nullptr) {
            throw std::runtime_error("cannot open " + out);
        }

        // copy input into output
        const int BUFFER_SIZE = 8192;
        char buffer[BUFFER_SIZE];
        fseeko(inFp, 0, SEEK_SET);
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
    if (inFp != nullptr) {
        fclose(inFp);
    }
    if (resFp != nullptr) {
        fclose(resFp);
    }
    if (outFp != nullptr) {
        fclose(outFp);
    }
    return ret;
}

bool ResourceAttacher::getResource(
    FILE* fp, uint64_t* offset) {

    fseeko(fp, 0, SEEK_END);
    uint64_t fileSize = ftello(fp);

    // validate footer data.
    if (fileSize < 24) {
        std::cerr << "invalid data." << std::endl;
        return false;
    }

    fseeko(fp, fileSize - 24, SEEK_SET);
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
