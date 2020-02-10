// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_PATCHGEN_PATCHFILE_FILELIST_H_
#define PATCHTOOL_PATCHGEN_PATCHFILE_FILELIST_H_

#include <filesystem>
#include <string>
#include <list>

struct File {
    std::string name;
    std::string fullFilename;
    std::string oldFilename;
    std::string newFilename;
    bool isDirectory;
    bool isAdd;
    bool isRemove;
    bool isModify;
    uint64_t filePos;
    uint64_t fileSize;
    uint32_t checkSum;

 public:
    File() :
        isDirectory(false), isAdd(false), isRemove(false), isModify(false),
        filePos(0), fileSize(0), checkSum(0) {}
};

struct FileList {
    std::string rootDir;
    std::list<File> files;
};

#endif  // PATCHTOOL_PATCHGEN_PATCHFILE_FILELIST_H_
