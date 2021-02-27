// Copyright (C) 2020 ISHIN.
#include <iostream>

extern "C" {
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #include <fcntl.h>
}

#if __GNUG__ <= 7
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#define FS_EXPERIMENTAL
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <regex>

#if WINDOWS
#include <windows.h>
#include <fileapi.h>
#endif

#include "include/patchtool.h"
#include "FileAccess.h"

#ifdef WINDOWS
#define TO_PATH(x) patchtool::FileAccess::charsToWchars((x))
#define TO_STR(x) patchtool::FileAccess::wcharsToChars((x))
#else
#define TO_PATH(x) (x)
#define TO_STR(x) (x)
#endif

namespace patchtool {

static std::regex reHiddenFiles("^\\..*");

FILE* FileAccess::openReadFile(const std::string& filePath) {
    return fopen(filePath.c_str(), "rb");
}

FILE* FileAccess::openWriteFile(const std::string& filePath) {
    return fopen(filePath.c_str(), "wb");
}

void FileAccess::closeFile(FILE* fp) {
    if (fp == nullptr) {
        return;
    }
    fclose(fp);
}

void FileAccess::seek(FILE* fp, uint64_t offset, int origin) {
    if (fp == nullptr) {
        return;
    }
    fseeko(fp, offset, origin);
}

uint64_t FileAccess::tell(FILE* fp) {
    if (fp == nullptr) {
        return 0;
    }
    return ftello(fp);
}

uint64_t FileAccess::read(FILE* fp, void* buffer, uint64_t size) {
    if (fp == nullptr) {
        return 0;
    }
    return fread(buffer, sizeof(char), size, fp);
}

bool FileAccess::readString(FILE* fp, std::string* value) {
    // fitst 2 bytes is string length.
    uint16_t length;
    if (read<uint16_t>(fp, &length) < 1) {
        return false;
    }

    // read string data.
    bool ret = true;
    char* buf = new char[length+1];
    memset(buf, 0, length + 1);
    if (read(fp, buf, length) == length) {
        *value = buf;
    } else {
        ret = false;
    }
    delete[] buf;
    return ret;
}

uint64_t FileAccess::write(FILE* fp, const void* buffer, uint64_t size) {
    if (fp == nullptr) {
        return 0;
    }
    return fwrite(buffer, sizeof(char), size, fp);
}

bool FileAccess::readBlock(
    FILE* fp, void* buffer, uint64_t* size, uint64_t blockSize) {
    bool ret = true;
    const int BUFFER_SIZE = 1024;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer);
    int readSize = 0;
    uint64_t totalRead = 0;
    do {
        if (totalRead + BUFFER_SIZE > blockSize) {
            readSize = blockSize - totalRead;
        } else {
            readSize = BUFFER_SIZE;
        }

        if (readSize > 0) {
            readSize = fread(ptr, sizeof(char), readSize, fp);
            if (readSize < 0) {
                std::cerr << "error occurred in reading. val="
                    << readSize << std::endl;
                ret = false;
                *size = 0;
                break;
            }
            totalRead += readSize;
            ptr += readSize;
        }
    } while (readSize > 0);

    if (ret) {
        *size = totalRead;
    }
    return ret;
}

bool FileAccess::writeBlock(
    FILE* fp, const void* buffer, uint64_t size) {
    bool ret = true;
    const int BUFFER_SIZE = 1024;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(const_cast<void*>(buffer));
    int writeSize = 0;
    uint64_t totalWrite = 0;
    do {
        if (totalWrite + BUFFER_SIZE > size) {
            writeSize = size - totalWrite;
        } else {
            writeSize = BUFFER_SIZE;
        }

        if (writeSize > 0) {
            writeSize = fwrite(ptr, sizeof(char), writeSize, fp);
            if (writeSize < 0) {
                std::cerr << "error occurred in reading. val="
                    << writeSize << std::endl;
                ret = false;
                break;
            }
            totalWrite += writeSize;
            ptr += writeSize;
        }
    } while (writeSize > 0);

    return ret;
}

FILE* FileAccess::createTempFile(
    const std::string& filePath, FILE* fp, uint64_t size) {

    FILE *writeFp = fopen(filePath.c_str(), "wb+");
    if (writeFp == nullptr) {
        std::cerr << "cannot create " << filePath << std::endl;
        return nullptr;
    }

    bool ret = true;
    const int BUFFER_SIZE = 1024;
    uint8_t* buf = new uint8_t[BUFFER_SIZE + 1];
    int readSize = 0;
    uint64_t totalRead = 0;
    do {
        if (totalRead + BUFFER_SIZE > size) {
            readSize = size - totalRead;
        } else {
            readSize = BUFFER_SIZE;
        }

        if (readSize > 0) {
            readSize = fread(buf, sizeof(char), readSize, fp);
            if (readSize < 0) {
                std::cerr << "error occurred in reading. val="
                    << readSize << std::endl;
                ret = false;
                break;
            }
            totalRead += readSize;
            fwrite(buf, sizeof(char), readSize, writeFp);
        }
    } while (readSize > 0);

    delete[] buf;

    if (!ret) {
        fclose(writeFp);
        removeFile(filePath);
        return nullptr;
    }

    fseeko(writeFp, 0, SEEK_SET);
    return writeFp;
}

uint32_t FileAccess::calcCheckSum(const std::string& filePath) {
    const int BLOCK_SIZE = 4096;
    uint8_t* buf = new uint8_t[BLOCK_SIZE+1];
    uint32_t checkSum = 0;
    FILE *fp = openReadFile(filePath);
    if (fp) {
        uint64_t readSize = 0;
        do {
            if (!readBlock(fp, buf, &readSize, BLOCK_SIZE)) {
                break;
            }
            uint8_t* ptr = buf;
            for (; ptr - buf < readSize; ptr++) {
                checkSum += *ptr;
            }
        } while (readSize > 0);
        closeFile(fp);
    }

    delete[] buf;
    return checkSum;
}

bool FileAccess::fileExists(const std::string& filePath) {
    return fs::exists(TO_PATH(filePath));
}

bool FileAccess::isDirectory(const std::string& filePath) {
    return fs::is_directory(TO_PATH(filePath));
}

bool FileAccess::isFile(const std::string& filePath) {
    return fs::is_regular_file(TO_PATH(filePath));
}

bool FileAccess::isHiddenFile(const std::string& filePath) {
#if WINDOWS
    // check hidden pattern
    uint32_t attr = GetFileAttributesA(filePath.c_str());
    return attr & 0x2;
#else
    return std::regex_match(filePath, reHiddenFiles);
#endif
}

const std::string FileAccess::getRelativePath(
    const std::string& filePath,
    const std::string& basePath) {
#ifdef FS_EXPERIMENTAL
    fs::path base(basePath);
    std::string convertedBasePath = TO_STR(base);

    // calc relative path from rootDir
    std::string relativePath = filePath;
    relativePath.replace(0, convertedBasePath.size(), "");
#if WINDOWS
    if (relativePath.at(0) == '\\') {
        relativePath.replace(0, 1, "");
    }
#else
    if (relativePath.at(0) == '/') {
        relativePath.replace(0, 1, "");
    }
#endif
    return relativePath;
#else
    return TO_STR(fs::relative(
        TO_PATH(filePath), TO_PATH(basePath)));
#endif
}

uint64_t FileAccess::getFileSize(const std::string& filePath) {
    return fs::file_size(TO_PATH(filePath));
}

bool FileAccess::copyFile(
    const std::string& srcPath,
    const std::string& destPath) {
    const int BUFFER_SIZE = 1024;
    char buf[BUFFER_SIZE + 1];

    FILE* fp1 = fopen(srcPath.c_str(), "rb");
    if (fp1 == nullptr) {
        std::cerr << "cannot open " << srcPath << std::endl;
        return false;
    }
    FILE* fp2 = fopen(destPath.c_str(), "wb");
    if (fp2 == nullptr) {
        std::cerr << "cannot open " << destPath << std::endl;
        fclose(fp1);
        return false;
    }

    bool ret = true;
    int readSize = 0;
    do {
        readSize = fread(buf, sizeof(char), BUFFER_SIZE, fp1);
        if (readSize < 0) {
            std::cerr << "error occurred in reading. val="
                << readSize << std::endl;
            ret = false;
            break;
        }
        fwrite(buf, sizeof(char), readSize, fp2);
    } while (readSize > 0);

    fclose(fp1);
    fclose(fp2);

    if (!ret) {
        removeFile(destPath);
        return false;
    }
    return true;
}

bool FileAccess::renameFile(
    const std::string& srcPath,
    const std::string& destPath) {
    try {
        fs::rename(TO_PATH(srcPath), TO_PATH(destPath));
    } catch (fs::filesystem_error& ex) {
        std::cerr << "cannot rename "
            << srcPath << " to " << destPath << std::endl;
        return false;
    }
    return true;
}

bool FileAccess::removeFile(const std::string& filePath) {
    try {
        fs::remove(TO_PATH(filePath));
    } catch (fs::filesystem_error& ex) {
        std::cerr << "cannot remove " << filePath << std::endl;
        return false;
    }
    return true;
}


bool FileAccess::createDirectory(const std::string& filePath) {
    try {
        fs::create_directory(TO_PATH(filePath));
    } catch (fs::filesystem_error& ex) {
        std::cerr << "cannot create " << filePath << std::endl;
        return false;
    }
    return true;
}

bool FileAccess::isFileEqual(
    const std::string& file1, const std::string& file2) {
    FILE *fp1, *fp2;
    const int BUFFER_SIZE = 1024;
    char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE];

    fp1 = fopen(file1.c_str(), "rb");
    if (fp1 == nullptr) {
        std::cerr << "cannot open " << file1 << std::endl;
        return false;
    }
    fp2 = fopen(file2.c_str(), "rb");
    if (fp2 == nullptr) {
        std::cerr << "cannot open " << file2 << std::endl;
        fclose(fp1);
        return false;
    }

    bool isEqual = true;
    int readCount1, readCount2;
    bool isContinue = true;
    char *ptr1, *ptr2;
    do {
        readCount1 = fread(buf1, sizeof(char), BUFFER_SIZE, fp1);
        readCount2 = fread(buf2, sizeof(char), BUFFER_SIZE, fp2);
        isContinue = (readCount1 == BUFFER_SIZE) && (readCount2 == BUFFER_SIZE);
        ptr1 = buf1;
        ptr2 = buf2;
        if (readCount1 == 0 && readCount2 > 0) {
            isEqual = false;
        }
        while (readCount1-- > 0) {
            if (readCount2-- == 0) {
                isEqual = false;
                break;
            }
            if (*ptr1++ != *ptr2++) {
                isEqual = false;
                isContinue = false;
                break;
            }
        }
    } while (isContinue);

    fclose(fp1);
    fclose(fp2);

    return isEqual;
}

std::list<std::string> FileAccess::searchDirectory(
    const std::string& filePath) {
    std::list<std::string> result;
    for (auto& entry : fs::directory_iterator(filePath)) {
        const std::string& path = TO_STR(entry.path());
        result.push_back(path);
    }
    return result;
}

#ifdef WINDOWS
std::wstring FileAccess::charsToWchars(const std::string& in) {
    int len = MultiByteToWideChar(CP_ACP, 0, in.c_str(), -1, nullptr, 0);
    wchar_t* buf = new wchar_t[len+1];
    std::wstring output;
    memset(buf, 0, sizeof(wchar_t) * (len+1));
    if (MultiByteToWideChar(CP_ACP, 0, in.c_str(), -1, buf, len)) {
        output = buf;
    } else {
        std::cerr << "code convert error." << std::endl;
    }
    delete[] buf;

    return output;
}

std::string FileAccess::wcharsToChars(const std::wstring& in) {
    int len = WideCharToMultiByte(
        CP_ACP, 0, in.c_str(), -1, nullptr, 0, nullptr, nullptr);
    char* buf = new char[(len+1) * 2];
    std::string output;
    memset(buf, 0, sizeof(char) * (len+1) * 2);
    if (WideCharToMultiByte(
        CP_ACP, 0, in.c_str(), -1, buf, len, nullptr, nullptr)) {
        output = buf;
    } else {
        std::cerr << "code convert error." << std::endl;
    }
    delete[] buf;

    return output;
}

#endif

}  // namespace patchtool
