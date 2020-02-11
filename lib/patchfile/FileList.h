// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_LIB_PATCHFILE_FILELIST_H_
#define PATCHTOOL_LIB_PATCHFILE_FILELIST_H_

#include <filesystem>
#include <string>
#include <list>

#ifdef WINDOWS
#define TO_PATH(x) File::charsToWchars((x))
#define TO_STR(x) File::wcharsToChars((x))
#else
#define TO_PATH(x) (x)
#define TO_STR(x) (x)
#endif

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

 public:
    File() :
        isDirectory(false), isAdd(false), isRemove(false), isModify(false),
        filePos(0), fileSize(0), checkSum(0) {}

    static bool isEqual(const File& file1, const File& file2);
    static std::wstring charsToWchars(const std::string& in);
    static std::string wcharsToChars(const std::wstring& in);
    uint16_t encodeFlags();
    void decodeFlags(uint16_t flags);
};

class FileList {
 public:
    std::string rootDir;
    std::list<File> files;

    void sortAsc();
    void dump();
    void search(const std::string& path);
    static FileList calcDiff(const FileList& oldList, const FileList& newList);
};

#endif  // PATCHTOOL_LIB_PATCHFILE_FILELIST_H_
