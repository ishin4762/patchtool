// Copyright (C) 2020 ISHIN.

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <filesystem>
#include <iostream>
#include "PatchFile.h"

namespace fs = std::filesystem;

/**
 * search diff between old-dir and new-dir.
 */
FileList PatchFile::searchDiff(
    const std::string& oldDir, const std::string& newDir) {
    FileList oldFileList, newFileList;
    oldFileList.rootDir = oldDir;
    oldFileList.search(oldDir);
    oldFileList.sortAsc();

    newFileList.rootDir = newDir;
    newFileList.search(newDir);
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
    }
}

void PatchFile::writeAdditionalFile(const File& file) {
    FILE *newFp = fopen(file.newFilename.c_str(), "rb");
    if (newFp == nullptr) {
        return;
    }

    const int BUFFER_SIZE = 1024;
    char buf[BUFFER_SIZE];
    int readCount;
    do {
        readCount = fread(buf, sizeof(char), BUFFER_SIZE, newFp);
        writeStream.write(&writeStream, buf, readCount);
    } while (readCount == BUFFER_SIZE);

    fclose(newFp);
}

void PatchFile::writeUpdateFile(File* file) {
    // open old file.
    int oldFd = open(file->oldFilename.c_str(), O_RDONLY, 0);
    if (oldFd < 0) {
        return;
    }

    // open new file.
    int newFd = open(file->newFilename.c_str(), O_RDONLY, 0);
    if (newFd < 0) {
        close(oldFd);
        return;
    }

    // read old file.
    off_t oldFileSize = lseek(oldFd, 0, SEEK_END);
    lseek(oldFd, 0, SEEK_SET);
    uint8_t* bufOld = new uint8_t[oldFileSize+1];
    read(oldFd, bufOld, oldFileSize);
    close(oldFd);

    // calc checksum.
    uint32_t checkSum = 0;
    uint8_t* ptrOld = bufOld;
    for (; ptrOld - bufOld < oldFileSize; ptrOld++) {
        checkSum += *ptrOld;
    }
    file->checkSum = checkSum;

    // read new file.
    off_t newFileSize = lseek(newFd, 0, SEEK_END);
    lseek(newFd, 0, SEEK_SET);
    uint8_t* bufNew = new uint8_t[newFileSize+1];
    read(newFd, bufNew, newFileSize);
    close(newFd);

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
            if (!fs::exists(filePath)) {
                std::cerr << (entry.isDirectory ? "directory" : "file")
                    << " " << entry.name << " is not found." << std::endl;
                return false;
            }
        }
        if (entry.isModify && !entry.isDirectory) {
            // check checksum.
            int fd = open(filePath.c_str(), O_RDONLY, 0);
            if (fd < 0) {
                std::cerr << "cannot open "
                    << " " << entry.name << std::endl;
                return false;
            }

            off_t fileSize = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            uint8_t* buf = new uint8_t[fileSize+1];
            read(fd, buf, fileSize);
            close(fd);

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
        if (entry.isDirectory) {
            if (entry.isAdd) {
                std::string filePath = fileList.rootDir + "/" + entry.name;
                try {
                    fs::create_directory(filePath);
                } catch (fs::filesystem_error& ex) {
                    std::cerr << "cannot create " << entry.name << std::endl;
                    return false;
                }
            } else if (entry.isRemove) {
                std::string filePath = fileList.rootDir + "/" + entry.name;
                try {
                    fs::remove_all(filePath);
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
                    fs::remove(filePath);
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
    FILE *newFp = fopen(writePath.c_str(), "wb");
    if (newFp == nullptr) {
        std::cerr << "cannot open " << writePath << std::endl;
        return false;
    }

    const int BUFFER_SIZE = 1024;
    char buf[BUFFER_SIZE];
    int readSize, writeCount;
    size_t totalRead = 0;
    do {
        if (totalRead + BUFFER_SIZE > file.fileNewSize) {
            readSize = file.fileNewSize - totalRead;
        } else {
            readSize = BUFFER_SIZE;
        }

        if (readSize > 0) {
            readStream.read(&readStream, buf, readSize);
            totalRead += readSize;
            writeCount = fwrite(buf, sizeof(char), readSize, newFp);
        }
    } while (readSize > 0);

    fclose(newFp);
    return true;
}

bool PatchFile::updateFile(
    const std::string& writePath, const File& file) {
    // open old file.
    int oldFd = open(writePath.c_str(), O_RDONLY, 0);
    if (oldFd < 0) {
        std::cerr << "cannot open " << writePath << std::endl;
        return false;
    }

    // read old file.
    off_t oldFileSize = lseek(oldFd, 0, SEEK_END);
    lseek(oldFd, 0, SEEK_SET);
    uint8_t* bufOld = new uint8_t[oldFileSize+1];
    read(oldFd, bufOld, oldFileSize);
    struct stat st;
    fstat(oldFd, &st);
    close(oldFd);

    // allocate new file buffer.
    off_t newFileSize = file.fileNewSize;
    uint8_t* bufNew = new uint8_t[newFileSize+1];

    // call bspatch.
    bspatch(bufOld, oldFileSize, bufNew, newFileSize, &readStream);

    // write file.
    oldFd = open(writePath.c_str(), O_CREAT|O_TRUNC|O_WRONLY, st.st_mode);
    if (oldFd < 0) {
        std::cerr << "cannot write " << writePath << std::endl;
        delete[] bufOld;
        delete[] bufNew;
        return false;
    }
    write(oldFd, bufNew, newFileSize);
    close(oldFd);

    delete[] bufOld;
    delete[] bufNew;

    return true;
}
