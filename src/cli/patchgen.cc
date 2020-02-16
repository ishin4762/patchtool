// Copyright(C) 2020 ISHIN.
extern "C" {
    #include <getopt.h>
}
#include <iostream>
#include <string>
#include <vector>
#include "patchfile/PatchFileFactory.h"
#include "patchfile/ResourceAttacher.h"

/**
 *  show usage.
 */
static void show_usage() {
    std::cout
        << "usage: patchgen [OPTION] old-dir new-dir output"
        << std::endl << std::endl
        << "OPTION:" << std::endl
        << "    -n, --no-compress        : no-compress mode (debug)"
        << std::endl
        << "    -e, --executable         : generate executable patch"
        << std::endl
        << "    -h, --add-hidden         : add hidden files."
        << std::endl
        << "    -i, --ignore pattern     : ignore regex pattern matched files."
        << std::endl;
}

/**
 *  entry point.
 */
int main(int argc, char* argv[]) {
    int opt;
    int long_index;
    bool nopt_available = false;
    bool eopt_available = false;
    bool hopt_available = false;
    bool iopt_available = false;
    std::string ignore_param;
    std::vector<std::string> nonopt_args;

    struct option longopts[] = {
        {"no-compress", no_argument,       NULL, 'n'},
        {"executable",  no_argument,       NULL, 'e'},
        {"add-hidden",  no_argument,       NULL, 'h'},
        {"ignore",      required_argument, NULL, 'i'},
        {0, 0, 0, 0}
    };

    // parse arguments.
    while ((opt = getopt_long(argc, argv,
        "nehi:", longopts, &long_index)) != -1) {
        switch (opt) {
        case 'n':
            nopt_available = true;
            break;

        case 'e':
            eopt_available = true;
            break;

        case 'h':
            hopt_available = true;
            break;

        case 'i':
            iopt_available = true;
            std::cerr << optarg << std::endl;
            ignore_param = optarg;
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

    // check if targets are directories.
    fs::path oldDir(TO_PATH(nonopt_args[0]));
    fs::path newDir(TO_PATH(nonopt_args[1]));
    if (!fs::exists(oldDir) || !fs::is_directory(oldDir)) {
        std::cout << nonopt_args[0] << " is not directory." << std::endl;
        return 1;
    } else if (!fs::exists(newDir) || !fs::is_directory(newDir)) {
        std::cout << nonopt_args[1] << " is not directory." << std::endl;
        return 1;
    } else if (fs::canonical(oldDir) == fs::canonical(newDir)) {
        std::cout << "old-dir and new-dir must be different directories."
            << std::endl;
        return 1;
    }

    PatchFile *patchFile = PatchFileFactory::create(
        nopt_available ? "" : "bzip2");

    // generate patch file.
    if (patchFile == nullptr
        || !patchFile->encode(
            nonopt_args[0], nonopt_args[1], nonopt_args[2],
            hopt_available, ignore_param)) {
        return 1;
    }

    if (eopt_available) {
        fs::path execDir(argv[0]);
#ifdef WINDOWS
        std::string baseExec =
            TO_STR(execDir.parent_path());
        baseExec += "selfapply.exe";
        std::string outputExec = nonopt_args[2] + ".exe";
#else
        std::string baseExec =
            TO_STR(execDir.parent_path());
        baseExec += "selfapply";
        std::string outputExec = nonopt_args[2] + ".out";
#endif
        // copy file to set attributes.
        try {
            fs::copy(
                fs::path(TO_PATH(baseExec)),
                fs::path(TO_PATH(outputExec)));
        } catch (const fs::filesystem_error& ex) {
            std::cerr << "cannot create " << outputExec << std::endl;
            return 1;
        }

        if (!ResourceAttacher::attach(
            baseExec, outputExec, nonopt_args[2])) {
            std::cerr << "cannot attach resource to " << outputExec
                << std::endl;
            return 1;
        }
    }
    return 0;
}
