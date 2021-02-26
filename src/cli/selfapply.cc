// Copyright(C) 2020 ISHIN.
extern "C" {
    #include <getopt.h>
#if !WINDOWS
    #include <unistd.h>
    #include <libproc.h>
#endif
}
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

#ifdef WINDOWS
#include <windows.h>
#endif

#include "lib/include/patchtool.h"
#include "common.h"

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
    patchtool::Patch patch;
    uint64_t offset;
    if (!patch.getResource(fp, &offset)) {
        fclose(fp);
        return 1;
    }

    std::string targetDir = argv[1];
    std::string lastChar = targetDir.substr(targetDir.size()-1, 1);
    if (lastChar == "/" || lastChar == "\\") {
        targetDir.pop_back();
    }

    fseeko(fp, offset, SEEK_SET);

    // apply patch file.
    if (!patch.decode(targetDir, fp, offset, false)) {
        std::cerr << "cannot apply patch." << std::endl;
        fclose(fp);
        return 1;
    }

    fclose(fp);

    std::cout << "succeeded." << std::endl;
    return 0;
}
