// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_PATCHGEN_PATCHFILE_FILELIST_H_
#define PATCHTOOL_PATCHGEN_PATCHFILE_FILELIST_H_

#include <filesystem>
#include <string>
#include <list>

struct File {
    std::string name;
    bool isDirectory;
    bool isAdd;
    bool isRemove;
    bool isModify;
    uint64_t filePos;
    uint64_t fileSize;

 public:
    File() :
        isDirectory(false), isAdd(false), isRemove(false), isModify(false),
        filePos(0), fileSize(0) {}
};

struct FileList {
    std::string rootDir;
    std::list<File> files;
};

#endif  // PATCHTOOL_PATCHGEN_PATCHFILE_FILELIST_H_
