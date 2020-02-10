// Copyright (C) 2020 ISHIN.
#ifndef PATCHTOOL_PATCHGEN_PATCHFILE_PATCHFILE_H_
#define PATCHTOOL_PATCHGEN_PATCHFILE_PATCHFILE_H_

#include <string>
#include "FileList.h"

#include "bsdiff/bsdiff.h"

class PatchFile {
 public:
    explicit PatchFile(const std::string& executableOS);
    virtual bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output) = 0;
    virtual bool decode() = 0;

 protected:
    std::string executableOS;
    bsdiff_stream stream;

    FileList searchDiff(const std::string& oldDir, const std::string& newDir);
    void writeFileInfo(FILE* fp, const FileList& fileList);

 private:
    void search(FileList* fileList, const std::string& path);
    void sortAsc(FileList* fileList);
    void dump(const FileList& fileList);
    FileList calcDiff(const FileList& oldList, const FileList& newList);
    bool isFileEqual(const File& file1, const File& file2);
    uint16_t encodeFlags(const File& file);
    void decodeFlags(uint16_t flags, File* file);
};

#endif  // PATCHTOOL_PATCHGEN_PATCHFILE_PATCHFILE_H_
