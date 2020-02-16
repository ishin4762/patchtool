// Copyright(C) 2020 ISHIN.
extern "C" {
    #include <getopt.h>
}
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include "patchfile/PatchFileFactory.h"
#include "patchfile/ResourceAttacher.h"

#ifdef WINDOWS
#include <windows.h>
#else
extern "C" {
    #include <unistd.h>
    #include <libproc.h>
}
#endif

/**
 *  show usage.
 */
static void show_usage(const std::string& prgName) {
    std::cout
        << "usage: " << prgName << " target-dir"
        << std::endl;
}

/**
 *  entry point.
 */
int main(int argc, char* argv[]) {
    // get program fullpath
    std::string prgFullPath;
#if WINDOWS
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
        std::cerr << "cannot open self." << std::endl;
    }
    prgFullPath = path;
#else
    fs::path exePath(TO_PATH(argv[0]));
    prgFullPath = TO_STR(fs::absolute(exePath));
#endif

    // get program name
    fs::path prgPath(TO_PATH(prgFullPath));
    std::string prgName = TO_STR(prgPath.filename());

    if (argc != 2) {
        show_usage(prgName);
        return 1;
    }

    // open self
    FILE* fp = fopen(prgFullPath.c_str(), "rb");
    if (fp == nullptr) {
        std::cerr << "cannot open " << prgName << std::endl;
        return 1;
    }

    // get resource
    uint64_t offset;
    if (!ResourceAttacher::getResource(fp, &offset)) {
        fclose(fp);
        return 1;
    }

    // generate patch data.
    PatchFile *patchFile = PatchFileFactory::fromFilePointer(fp, offset);

    std::string targetDir = argv[1];
    std::string lastChar = targetDir.substr(targetDir.size()-1, 1);
    if (lastChar == "/" || lastChar == "\\") {
        targetDir.pop_back();
    }

    fseeko(fp, offset, SEEK_SET);

    // apply patch file.
    if (patchFile == nullptr
        || !patchFile->decode(targetDir, fp, offset)) {
        std::cerr << "cannot apply patch." << std::endl;
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}
