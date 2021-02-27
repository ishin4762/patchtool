// Copyright (C) 2020 ISHIN.
#ifndef SRC_LIB_PATCHDECODER_H_
#define SRC_LIB_PATCHDECODER_H_

#include <string>
#include <unordered_map>

#include "PatchFile.h"
#include "FileList.h"
#include "FileAccess.h"

namespace patchtool {

class PatchDecoder {
 public:
    PatchDecoder(
        PatchFile* _patchFile,
        FileAccess* _fileAccess,
        FILE* _fp,
        uint64_t _offset,
        bool _isVerbose)
            : patchFile(_patchFile),
            fileAccess(_fileAccess),
            fp(_fp),
            offset(_offset),
            isVerbose(_isVerbose) {}
    bool decode(
        const std::string& targetDir);

 protected:
    bool checkSignature();
    bool readHeader(FileList* fileList);
    bool readFileInfo(File* file, int version);
    bool validateFiles(const FileList& fileList);
    bool validateFile(const std::string& baseDir, const File& file);
    bool applyFiles(const FileList& fileList);
    bool applyFile(
        const std::string& baseDir,
        const File& file,
        std::unordered_map<std::string, std::string>* nameMap);
    bool cleanupFiles(
        const std::string& baseDir, bool isSucceeded);
    bool cleanupFile(
        const std::string& path, bool isSucceeded);

    bool generateFile(
        const std::string& path, const File& file);
    bool updateFile(
        const std::string& path, const File& file);

    const std::string generateSuffix();
    bool stringEndsWith(
        const std::string& str, const std::string& suffix);
    const std::string trimSuffix(
        const std::string& str, const std::string& suffix);
    const std::string applyRename(
        std::unordered_map<std::string, std::string>* nameMap,
        const std::string& baseDir,
        const std::string& filename,
        const std::string& suffix,
        bool isAdd);

 private:
    PatchFile* patchFile;
    FileAccess* fileAccess;
    FILE *fp;
    uint64_t offset;
    bool isVerbose;
    std::string suffix;
};

}  // namespace patchtool

#endif  // SRC_LIB_PATCHDECODER_H_
