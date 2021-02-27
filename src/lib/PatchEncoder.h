// Copyright (C) 2020 ISHIN.
#ifndef SRC_LIB_PATCHENCODER_H_
#define SRC_LIB_PATCHENCODER_H_

#include <string>

#include "PatchFile.h"
#include "FileList.h"
#include "FileAccess.h"

namespace patchtool {

class PatchEncoder {
 public:
    PatchEncoder(
        PatchFile* _patchFile,
        FileAccess* _fileAccess,
        bool _isVerbose,
        uint32_t _blockSize)
            : patchFile(_patchFile),
            fileAccess(_fileAccess),
            isVerbose(_isVerbose),
            blockSize(_blockSize) {}
    bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output,
        bool isHiddenSearch,
        const std::string& ignorePattern);

 protected:
    void create(FILE* fp, FileList* fileList);
    void writeHeader(
        FILE* fp, FileList* fileList, bool isFirstCall);
    void writeFileInfo(
        FILE* fp, File* file, bool isFirstCall);
    void writeData(FILE* fp, FileList* fileList);
    void writeFile(FILE* fp, File* file);
    void writeAdditionalFile(FILE* fp, File* file);
    void writeUpdateFile(FILE* fp, File* file);

 private:
    PatchFile* patchFile;
    FileAccess* fileAccess;
    bool isVerbose;
    uint32_t blockSize;
};

}  // namespace patchtool

#endif  // SRC_LIB_PATCHENCODER_H_
