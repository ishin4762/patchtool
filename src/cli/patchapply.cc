// Copyright(C) 2020 ISHIN.
extern "C" {
    #include <getopt.h>
}
#include <iostream>
#include <string>
#include <vector>
#include "patchfile/PatchFileFactory.h"

/**
 *  show usage.
 */
static void show_usage() {
    std::cout
        << "usage: patchapply target-dir patch-file"
        << std::endl;
}

/**
 *  entry point.
 */
int main(int argc, char* argv[]) {
    if (argc != 3) {
        show_usage();
        return 1;
    }

    PatchFile *patchFile = PatchFileFactory::fromFile(argv[2]);

    std::string targetDir = argv[1];
    std::string lastChar = targetDir.substr(targetDir.size()-1, 1);
    if (lastChar == "/" || lastChar == "\\") {
        targetDir.pop_back();
    }

    // apply patch file.
    if (patchFile == nullptr
        || !patchFile->decode(targetDir, argv[2])) {
        std::cerr << "cannot apply patch." << std::endl;
        return 1;
    }

    return 0;
}
