// Copyright (C) 2020 ISHIN.
#ifndef LIB_PATCHFILE_PATCHFILE_H_
#define LIB_PATCHFILE_PATCHFILE_H_

#include <string>
#include "FileList.h"

extern "C" {
#include "lib/bsdiff/bsdiff.h"
#include "lib/bsdiff/bspatch.h"
}

class PatchFile {
 public:
    PatchFile() {}
    virtual bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output) = 0;
    virtual bool decode(
        const std::string& targetDir,
        const std::string& input) = 0;

 protected:
    bsdiff_stream writeStream;
    bspatch_stream readStream;
    char signature[16] = { 0 };

    virtual bool openWriter(FILE* fp) = 0;
    virtual bool closeWriter() = 0;
    virtual bool openReader(FILE* fp) = 0;
    virtual bool closeReader() = 0;

    FileList searchDiff(const std::string& oldDir, const std::string& newDir);
    void create(FILE* fp, FileList* fileList);
    bool apply(const std::string& targetDir, FILE* fp);

 private:
    void writeFileInfo(FILE* fp, const FileList& fileList);
    void writeFileData(FILE* fp, FileList* fileList);
    void writeAdditionalFile(const File& file);
    void writeUpdateFile(File* file);

    bool readFileInfo(FILE* fp, FileList* fileList);
    bool validateFiles(const FileList& fileList);
    bool applyFiles(FILE* fp, const FileList& fileList);
    bool generateFile(const std::string& writePath, const File& file);
    bool updateFile(const std::string& writePath, const File& file);

    bool readRawFile(
        const std::string& readPath, uint8_t** buf, uint64_t* size);
    bool writeRawFile(
        const std::string& writePath, uint8_t* buf, uint64_t size);
};

#endif  // LIB_PATCHFILE_PATCHFILE_H_
