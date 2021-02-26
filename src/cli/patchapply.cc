// Copyright(C) 2020 ISHIN.
extern "C" {
    #include <getopt.h>
}
#include <iostream>
#include <string>
#include <vector>

#include "lib/include/patchtool.h"
#include "common.h"

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

    std::string targetDir = argv[1];
    std::string lastChar = targetDir.substr(targetDir.size()-1, 1);
    if (lastChar == "/" || lastChar == "\\") {
        targetDir.pop_back();
    }

    // apply patch file.
    patchtool::Patch patch;
    if (!patch.decode(targetDir, argv[2], false)) {
        std::cerr << "cannot apply patch." << std::endl;
        return 1;
    }

    std::cout << "succeeded." << std::endl;
    return 0;
}
