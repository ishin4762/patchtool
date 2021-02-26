// Copyright (C) 2020 ISHIN.
#ifndef SRC_LIB_FILELIST_H_
#define SRC_LIB_FILELIST_H_

#include <regex>
#include <string>
#include <list>
#include <vector>

#include "FileAccess.h"

namespace patchtool {

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
    uint64_t fileNewSize;
    uint32_t checkSum;

    uint64_t numBlocks;
    std::vector<uint32_t> oldBlockSizeList;
    std::vector<uint32_t> newBlockSizeList;
    std::vector<uint32_t> diffBlockSizeList;

 public:
    File() :
        isDirectory(false), isAdd(false), isRemove(false), isModify(false),
        filePos(0), fileSize(0), checkSum(0), numBlocks(0) {}

#ifdef WINDOWS
    static std::wstring charsToWchars(const std::string& in);
    static std::string wcharsToChars(const std::wstring& in);
#endif
    uint16_t encodeFlags();
    void decodeFlags(uint16_t flags);
};

class FileList {
 public:
    std::string rootDir;
    std::list<File> files;

    void sortAsc();
    void dump();
    void search(
        FileAccess* fileAccess,
        const std::string& path,
        bool isHiddenSearch,
        bool isCheckIgnore,
        const std::regex& reIgnorePattern);
    static FileList searchDiff(
        FileAccess* fileAccess,
        const std::string& oldDir,
        const std::string& newDir,
        bool isHiddenSearch,
        const std::string& ignorePattern);
    static FileList calcDiff(
        FileAccess* fileAccess,
        const FileList& oldList,
        const FileList& newList);
};

}  // namespace patchtool

#endif  // SRC_LIB_FILELIST_H_
