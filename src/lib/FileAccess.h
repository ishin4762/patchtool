// Copyright (C) 2020 ISHIN.
#ifndef SRC_LIB_FILEACCESS_H_
#define SRC_LIB_FILEACCESS_H_

extern "C" {
  #include <stdio.h>
}

#include <string>
#include <list>

namespace patchtool {

class FileAccess {
 public:
    FileAccess() {}

    FILE* openReadFile(const std::string& filePath);
    FILE* openWriteFile(const std::string& filePath);
    void closeFile(FILE* fp);

    uint64_t tell(FILE* fp);
    void seek(FILE* fp, uint64_t offset, int origin);
    uint64_t read(FILE* fp, void* buffer, uint64_t size);
    bool readString(FILE* fp, std::string* value);
    uint64_t write(FILE* fp, const void* buffer, uint64_t size);

    template <typename T>
    uint64_t read(FILE* fp, T* value) {
        return read(fp, value, sizeof(T)) / sizeof(T);
    }
    template <typename T>
    uint64_t write(FILE* fp, T value) {
        return write(fp, &value, sizeof(T)) / sizeof(T);
    }

    bool readBlock(
        FILE* fp, void* buffer, uint64_t* size, uint64_t blockSize);
    bool writeBlock(
        FILE* fp, const void* buffer, uint64_t size);
    FILE* createTempFile(
        const std::string& filePath, FILE* fp, uint64_t size);

    uint32_t calcCheckSum(const std::string& filePath);
    bool fileExists(const std::string& filePath);
    bool isDirectory(const std::string& filePath);
    bool isFile(const std::string& filePath);
    bool isHiddenFile(const std::string& filePath);
    const std::string getRelativePath(
        const std::string& filePath, const std::string& basePath);
    uint64_t getFileSize(const std::string& filePath);
    bool createDirectory(const std::string& filePath);
    bool copyFile(
        const std::string& destPath,
        const std::string& srcPath);
    bool renameFile(
        const std::string& destPath,
        const std::string& srcPath);
    bool removeFile(const std::string& filePath);
    bool isFileEqual(
        const std::string& file1, const std::string& file2);
    std::list<std::string> searchDirectory(
        const std::string& filePath);

#ifdef WINDOWS
    static std::wstring charsToWchars(const std::string& in);
    static std::string wcharsToChars(const std::wstring& in);
#endif
};

}  // namespace patchtool

#endif  // SRC_LIB_FILEACCESS_H_
