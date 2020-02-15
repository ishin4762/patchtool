// Copyright (C) 2020 ISHIN.
extern "C" {
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>
}
#include <cstring>
#include <iostream>
#include "PatchFile.h"

/**
 * search diff between old-dir and new-dir.
 */
FileList PatchFile::searchDiff(
    const std::string& oldDir,
    const std::string& newDir,
    bool isHiddenSearch,
    const std::string& ignorePattern) {

    std::regex reIgnorePattern(ignorePattern);

    FileList oldFileList, newFileList;
    oldFileList.rootDir = oldDir;
    oldFileList.search(
        oldDir,
        isHiddenSearch,
        !ignorePattern.empty(),
        reIgnorePattern);
    oldFileList.sortAsc();

    newFileList.rootDir = newDir;
    newFileList.search(
        newDir,
        isHiddenSearch,
        !ignorePattern.empty(),
        reIgnorePattern);
    newFileList.sortAsc();

    FileList diffList = FileList::calcDiff(oldFileList, newFileList);
    diffList.dump();
    return diffList;
}

void PatchFile::create(FILE* fp, FileList* fileList) {
    // 0) write signature.
    fwrite(signature, sizeof(char), sizeof(signature), fp);

    // 1) write headers. (filePos & fileSize must be zero.)
    writeFileInfo(fp, *fileList);

    // 2) write data.
    std::cout << std::endl << "Writing..." << std::endl;
    writeFileData(fp, fileList);

    // 3) re-write headers. (filePos & fileSize will be non-zero.)
    writeFileInfo(fp, *fileList);
}

void PatchFile::writeFileInfo(FILE* fp, const FileList& fileList) {
    // move pointer to head.
    fseeko(fp, sizeof(signature), SEEK_SET);

    uint32_t numEntries = fileList.files.size();
    fwrite(&numEntries, sizeof(uint32_t), 1, fp);

    for (auto entry : fileList.files) {
        // filename
        uint16_t length = entry.name.length();
        fwrite(&length, sizeof(uint16_t), 1, fp);
        fwrite(entry.name.c_str(), sizeof(char), length, fp);

        // flags
        uint16_t flags = entry.encodeFlags();
        fwrite(&flags, sizeof(uint16_t), 1, fp);

        // filePos
        uint64_t filePos = entry.filePos;
        fwrite(&filePos, sizeof(uint64_t), 1, fp);

        // fileSize
        uint64_t fileSize = entry.fileSize;
        fwrite(&fileSize, sizeof(uint64_t), 1, fp);

        // fileNewSize
        uint64_t fileNewSize = entry.fileNewSize;
        fwrite(&fileNewSize, sizeof(uint64_t), 1, fp);

        // checksum
        uint32_t checkSum = entry.checkSum;
        fwrite(&checkSum, sizeof(uint32_t), 1, fp);
    }
}

void PatchFile::writeFileData(FILE* fp, FileList* fileList) {
    for (auto& entry : fileList->files) {
        if (entry.isDirectory || entry.isRemove) {
            continue;
        }

        std::cout << entry.name << " ...";

        if (entry.isAdd) {
            entry.filePos = ftello(fp);
            openWriter(fp);
            writeAdditionalFile(entry);
            closeWriter();
            entry.fileSize = ftello(fp) - entry.filePos;

        } else if (entry.isModify) {
            entry.filePos = ftello(fp);
            openWriter(fp);
            writeUpdateFile(&entry);
            closeWriter();
            entry.fileSize = ftello(fp) - entry.filePos;
        }

        if (entry.fileNewSize > 0) {
            int64_t percent = entry.fileSize * 100 / entry.fileNewSize;
            std::cout << entry.fileSize << "/" << entry.fileNewSize
                << " bytes (" << percent << "%)" << std::endl;
        } else {
           std::cout << entry.fileSize << "/" << entry.fileNewSize
                << " bytes (---%)" << std::endl;
        }
    }
}

void PatchFile::writeAdditionalFile(const File& file) {
    // open new file.
    uint64_t newFileSize;
    uint8_t* bufNew;
    if (!readRawFile(file.newFilename, &bufNew, &newFileSize)) {
        return;
    }

    // old file (dummy)
    uint64_t oldFileSize = 0;
    uint8_t* bufOld = new uint8_t[oldFileSize+1];

    // call bsdiff.
    bsdiff(bufOld, oldFileSize, bufNew, newFileSize, &writeStream);

    delete[] bufOld;
    delete[] bufNew;
}

void PatchFile::writeUpdateFile(File* file) {
    // open old file.
    uint64_t oldFileSize;
    uint8_t* bufOld;
    if (!readRawFile(file->oldFilename, &bufOld, &oldFileSize)) {
        return;
    }

    // open new file.
    uint64_t newFileSize;
    uint8_t* bufNew;
    if (!readRawFile(file->newFilename, &bufNew, &newFileSize)) {
        delete[] bufOld;
        return;
    }

    // calc checksum.
    uint32_t checkSum = 0;
    uint8_t* ptrOld = bufOld;
    for (; ptrOld - bufOld < oldFileSize; ptrOld++) {
        checkSum += *ptrOld;
    }
    file->checkSum = checkSum;

    // call bsdiff.
    bsdiff(bufOld, oldFileSize, bufNew, newFileSize, &writeStream);

    delete[] bufOld;
    delete[] bufNew;
}

bool PatchFile::apply(const std::string& targetDir, FILE* fp) {
    fseeko(fp, 0, SEEK_SET);

    // 0) skip signature.
    char dummy[16] = { 0 };
    if (fread(dummy, sizeof(char), 16, fp) < 16) {
        std::cerr << "invalid header." << std::endl;
        return false;
    }

    // 1) read headers.
    FileList fileList;
    fileList.rootDir = targetDir;
    if (!readFileInfo(fp, &fileList)) {
        std::cerr << "cannot read file info." << std::endl;
        return false;
    }

    // 2) validate file existence & checksum.
    if (!validateFiles(fileList)) {
        return false;
    }

    // 3) apply.
    std::cout << "Applying..." << std::endl;
    if (!applyFiles(fp, fileList)) {
        return false;
    }

    return true;
}

bool PatchFile::readFileInfo(FILE* fp, FileList* fileList) {
    // move pointer to head.
    if (fseeko(fp, sizeof(signature), SEEK_SET) != 0) {
        return false;
    }

    // get # of entries.
    uint32_t numEntries;
    if (fread(&numEntries, sizeof(uint32_t), 1, fp) < 1) {
        return false;
    }

    for (uint32_t i=0; i < numEntries; i++) {
        File file;

        // filename
        uint16_t length;
        if (fread(&length, sizeof(uint16_t), 1, fp) < 1) {
            return false;
        }
        char* buf = new char[length+1];
        memset(buf, 0, length + 1);
        if (fread(buf, sizeof(char), length, fp) < length) {
            return false;
        }
        file.name = buf;
        delete[] buf;

        // flags
        uint16_t flags;
        if (fread(&flags, sizeof(uint16_t), 1, fp) < 1) {
            return false;
        }
        file.decodeFlags(flags);

        // filePos
        uint64_t filePos;
        if (fread(&filePos, sizeof(uint64_t), 1, fp) < 1) {
            return false;
        }
        file.filePos = filePos;

        // fileSize
        uint64_t fileSize;
        if (fread(&fileSize, sizeof(uint64_t), 1, fp) < 1) {
            return false;
        }
        file.fileSize = fileSize;

        // fileNewSize
        uint64_t fileNewSize;
        if (fread(&fileNewSize, sizeof(uint64_t), 1, fp) < 1) {
            return false;
        }
        file.fileNewSize = fileNewSize;

        // checksum
        uint32_t checkSum;
        if (fread(&checkSum, sizeof(uint32_t), 1, fp) < 1) {
            return false;
        }
        file.checkSum = checkSum;

        fileList->files.push_back(file);
    }

    return true;
}

bool PatchFile::validateFiles(const FileList& fileList) {
    for (const auto& entry : fileList.files) {
        std::string filePath;
        if (entry.isRemove || entry.isModify) {
            // check if file or directory exists.
            filePath = fileList.rootDir + "/" + entry.name;
            if (!fs::exists(TO_PATH(filePath))) {
                std::cerr << (entry.isDirectory ? "directory" : "file")
                    << " " << entry.name << " is not found." << std::endl;
                return false;
            }
        }
        if (entry.isModify && !entry.isDirectory) {
            // check checksum.
            uint64_t fileSize;
            uint8_t* buf;
            if (!readRawFile(filePath, &buf, &fileSize)) {
                return false;
            }

            // calc checksum.
            uint32_t checkSum = 0;
            uint8_t* ptr = buf;
            for (; ptr - buf < fileSize; ptr++) {
                checkSum += *ptr;
            }
            delete[] buf;

            if (checkSum != entry.checkSum) {
                std::cerr << "checksum " << entry.name
                    << " is not match." << std::endl;
                return false;
            }
        }
    }

    return true;
}


bool PatchFile::applyFiles(FILE* fp, const FileList& fileList) {
    if (fileList.rootDir.empty()) {
        std::cerr << "invalid target dir." << std::endl;
        return false;
    }

    for (auto& entry : fileList.files) {
        std::cout << entry.name << std::endl;

        if (entry.isDirectory) {
            if (entry.isAdd) {
                std::string filePath = fileList.rootDir + "/" + entry.name;
                try {
                    fs::create_directory(TO_PATH(filePath));
                } catch (fs::filesystem_error& ex) {
                    std::cerr << "cannot create " << entry.name << std::endl;
                    return false;
                }
            } else if (entry.isRemove) {
                std::string filePath = fileList.rootDir + "/" + entry.name;
                try {
                    fs::remove_all(TO_PATH(filePath));
                } catch (fs::filesystem_error& ex) {
                    std::cerr << "cannot remove " << entry.name << std::endl;
                    return false;
                }
            }
        } else {
            if (entry.isAdd) {
                std::string filePath = fileList.rootDir + "/" + entry.name;
                fseeko(fp, entry.filePos, SEEK_SET);
                openReader(fp);
                generateFile(filePath, entry);
                closeReader();
            } else if (entry.isRemove) {
                std::string filePath = fileList.rootDir + "/" + entry.name;
                try {
                    fs::remove(TO_PATH(filePath));
                } catch (fs::filesystem_error& ex) {
                    std::cerr << "cannot remove " << entry.name << std::endl;
                    return false;
                }
            } else if (entry.isModify) {
                std::string filePath = fileList.rootDir + "/" + entry.name;
                fseeko(fp, entry.filePos, SEEK_SET);
                openReader(fp);
                updateFile(filePath, entry);
                closeReader();
            }
        }
    }

    return true;
}

bool PatchFile::generateFile(
    const std::string& writePath, const File& file) {
    // allocate new file buffer.
    off_t newFileSize = file.fileNewSize;
    uint8_t* bufNew = new uint8_t[newFileSize+1];
    memset(bufNew, 0, newFileSize + 1);

    // old file (dummy)
    off_t oldFileSize = 0;
    uint8_t* bufOld = new uint8_t[oldFileSize+1];

    // call bspatch.
    bspatch(bufOld, oldFileSize, bufNew, newFileSize, &readStream);

    // write file.
    bool ret = writeRawFile(writePath, bufNew, newFileSize);
    delete[] bufOld;
    delete[] bufNew;

    return ret;
}

bool PatchFile::updateFile(
    const std::string& writePath, const File& file) {
    // open old file.
    uint64_t oldFileSize;
    uint8_t* bufOld;
    if (!readRawFile(writePath, &bufOld, &oldFileSize)) {
        return false;
    }

    // allocate new file buffer.
    uint64_t newFileSize = file.fileNewSize;
    uint8_t* bufNew = new uint8_t[newFileSize+1];

    // call bspatch.
    bspatch(bufOld, oldFileSize, bufNew, newFileSize, &readStream);

    // write file.
    bool ret = writeRawFile(writePath, bufNew, newFileSize);
    delete[] bufOld;
    delete[] bufNew;

    return ret;
}

bool PatchFile::readRawFile(
    const std::string& readPath, uint8_t** buf, uint64_t* size) {

    FILE *fp = fopen(readPath.c_str(), "rb");
    if (fp == nullptr) {
        std::cerr << "cannot read " << readPath << std::endl;
        return false;
    }

    // get file length.
    fseeko(fp, 0, SEEK_END);
    *size = ftello(fp);
    fseeko(fp, 0, SEEK_SET);

    // allocate buffer
    *buf = new uint8_t[*size];

    bool ret = true;
    const int BUFFER_SIZE = 1024;
    uint8_t* ptr = *buf;
    int readSize = 0;
    uint64_t totalRead = 0;
    do {
        if (totalRead + BUFFER_SIZE > *size) {
            readSize = *size - totalRead;
        } else {
            readSize = BUFFER_SIZE;
        }

        if (readSize > 0) {
            readSize = fread(ptr, sizeof(char), readSize, fp);
            if (readSize < 0) {
                std::cerr << "error occurred in reading. val="
                    << readSize << std::endl;
                ret = false;
                delete[] *buf;
                *size = 0;
                break;
            }
            totalRead += readSize;
            ptr += readSize;
        }
    } while (readSize > 0);

    fclose(fp);

    return ret;
}

bool PatchFile::writeRawFile(
    const std::string& writePath, uint8_t* buf, uint64_t size) {

    FILE *fp = fopen(writePath.c_str(), "wb");
    if (fp == nullptr) {
        std::cerr << "cannot write " << writePath << std::endl;
        return false;
    }

    bool ret = true;
    const int BUFFER_SIZE = 1024;
    uint8_t* ptr = buf;
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
                std::cerr << "error occurred in writing. val="
                    << writeSize << std::endl;
                ret = false;
                break;
            }
            totalWrite += writeSize;
            ptr += writeSize;
        }
    } while (writeSize > 0);

    fclose(fp);
    return ret;
}
