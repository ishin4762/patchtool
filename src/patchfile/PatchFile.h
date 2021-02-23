// Copyright (C) 2020 ISHIN.
#ifndef SRC_PATCHFILE_PATCHFILE_H_
#define SRC_PATCHFILE_PATCHFILE_H_

#include <string>
#include <unordered_map>
#include "FileList.h"

extern "C" {
#include "lib/bsdiff/bsdiff.h"
#include "lib/bsdiff/bspatch.h"
}

class PatchFile {
 public:
    PatchFile() {}
    bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output,
        bool isHiddenSearch,
        const std::string& ignorePattern,
        uint32_t blockSize);
    bool decode(
        const std::string& targetDir,
        const std::string& input);
    bool decode(
        const std::string& targetDir,
        FILE *fp,
        const uint64_t offset);
    void setVerbose(bool value) {
        isVerbose = value;
    }

 protected:
    bsdiff_stream writeStream;
    bspatch_stream readStream;
    char signature[16] = { 0 };
    uint64_t fileOffset = 0;
    bool isVerbose = false;

    virtual bool openWriter(FILE* fp) = 0;
    virtual bool closeWriter() = 0;
    virtual bool openReader(FILE* fp) = 0;
    virtual bool closeReader() = 0;

    FileList searchDiff(
        const std::string& oldDir,
        const std::string& newDir,
        bool isHiddenSearch,
        const std::string& ignorePattern);
    void create(FILE* fp, FileList* fileList, uint32_t blockSize);
    bool apply(const std::string& targetDir, FILE* fp);

 private:
    void writeFileInfo(
        FILE* fp, FileList* fileList, uint32_t blockSize, bool isFirstCall);
    void writeFileData(FILE* fp, FileList* fileList, uint32_t blockSize);
    void writeAdditionalFile(
        FILE* writeFp, File* file, uint32_t blockSize);
    void writeUpdateFile(
        FILE* writeFp, File* file, uint32_t blockSize);

    uint32_t calcCheckSum(const std::string& filePath);
    bool readFileInfo(FILE* fp, FileList* fileList);
    bool validateFiles(const FileList& fileList);
    bool applyFiles(
        FILE* fp, const FileList& fileList, const std::string& suffix);
    bool cleanupFiles(
        const std::string& baseDir,
        const std::string& suffix, bool isSucceeded);
    bool generateFile(
        FILE* readFp, const std::string& writePath, const File& file);
    bool updateFile(
        FILE* readFp, const std::string& writePath, const File& file);

    bool readRawFile(
        const std::string& readPath, uint8_t** buf, uint64_t* size);
    bool writeRawFile(
        const std::string& writePath, uint8_t* buf, uint64_t size);
    bool readRawFilePartial(
        FILE** fp, const std::string& readPath,
        uint8_t* buf, uint64_t* size, uint32_t blockSize);
    bool writeRawFilePartial(
        FILE** fp, const std::string& writePath,
        uint8_t* buf, uint64_t size);
    FILE* createTempFile(
        FILE* baseFp, uint64_t size, const std::string& filePath);

    const std::string generateSuffix();
    bool stringEndsWith(const std::string& str, const std::string& suffix);
    const std::string trimSuffix(
        const std::string& str, const std::string& suffix);
    std::string applyNameMap(
        const std::unordered_map<std::string, std::string>& map,
        const std::string& before, const std::string& suffix);
};

#endif  // SRC_PATCHFILE_PATCHFILE_H_
