// Copyright(C) 2020 ISHIN.
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include "PatchFile/PatchFileFactory.h"

/**
 *  show usage.
 */
static void show_usage() {
    std::cout
        << "usage: patchgen [OPTION] old-dir new-dir output"
        << std::endl << std::endl
        << "OPTION:" << std::endl
        << "    -c, --compress (bzip2|zlib)  : compress mode"
        << std::endl
        << "    -e, --executable (win|mac)   : generate executable patch"
        << std::endl;
}

/**
 *  entry point.
 */
int main(int argc, char* argv[]) {
    int opt;
    int long_index;
    bool copt_available = false;
    bool eopt_available = false;
    std::string compress_param;
    std::string executable_param;
    std::vector<std::string> nonopt_args;

    struct option longopts[] = {
        {"compress", required_argument, NULL, 'c'},
        {"executable", required_argument, NULL, 'e'},
        {0, 0, 0, 0}
    };

    // parse arguments.
    while ((opt = getopt_long(argc, argv, "c:", longopts, &long_index)) != -1) {
        switch (opt) {
        case 'c':
            copt_available = true;
            compress_param = optarg;
            break;

        case 'e':
            eopt_available = true;
            executable_param = optarg;
            break;

        default:
            show_usage();
            return 1;
        }
    }

    // parse rest arguments.
    while (optind < argc) {
        nonopt_args.push_back(argv[optind++]);
    }
    if (nonopt_args.size() != 3) {
        show_usage();
        return 1;
    }

    // validate arguments.
    if (compress_param != "bzip2"
        && compress_param != "zlib"
        && !compress_param.empty()) {
        show_usage();
        return 1;
    } else if (executable_param != "win"
        && executable_param != "mac"
        && !compress_param.empty()) {
        show_usage();
        return 1;
    }

    // not implemented error.
    // TODO(ishin): remove these validations after implements.
    if (compress_param == "bzip2") {
        std::cout << "sorry, bzip2 compress mode is not implemented."
            << std::endl;
        return 1;
    } else if (compress_param == "zlib") {
        std::cout << "sorry, zlib compress mode is not implemented."
            << std::endl;
        return 1;
    }
    if (executable_param == "win") {
        std::cout << "sorry, win executable mode is not implemented."
            << std::endl;
        return 1;
    } else if (executable_param == "mac") {
        std::cout << "sorry, mac executable mode is not implemented."
            << std::endl;
        return 1;
    }

    PatchFile *patchFile = PatchFileFactory::create(
        compress_param, executable_param);

    // generate patch file.
    if (patchFile == nullptr
        || !patchFile->encode(
            nonopt_args[0], nonopt_args[1], nonopt_args[2])) {
        return 1;
    }

    return 0;
}
