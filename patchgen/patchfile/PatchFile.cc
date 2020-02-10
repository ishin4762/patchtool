// Copyright (C) 2020 ISHIN.
#include <filesystem>
#include <iostream>
#include "PatchFile.h"

namespace fs = std::filesystem;

/**
 * constructor.
 */
PatchFile::PatchFile(const std::string& executableOS) :
    executableOS(executableOS) {
}

/**
 * search diff between old-dir and new-dir.
 */
FileList PatchFile::searchDiff(
    const std::string& oldDir, const std::string& newDir) {
    FileList oldFileList, newFileList;
    oldFileList.rootDir = oldDir;
    search(&oldFileList, oldDir);
    sortAsc(&oldFileList);
    newFileList.rootDir = newDir;
    search(&newFileList, newDir);
    sortAsc(&newFileList);

    FileList diffList = calcDiff(oldFileList, newFileList);
    dump(diffList);
    return diffList;
}

/**
 * search and enumerate files and dirs.
 */
void PatchFile::search(
    FileList* fileList, const std::string& path) {

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_directory()) {
            search(fileList, entry.path());
        }

        File file;
        fs::path path = entry.path();
        fs::path basePath(fileList->rootDir);
        file.name = fs::relative(path, basePath).generic_string();
        file.isDirectory = entry.status().type() == fs::file_type::directory;
        fileList->files.push_back(file);
    }
}

/**
 * internal compare function.
 */
static bool __fileComp(const File& v1, const File& v2) {
    if (v1.name < v2.name) {
        return true;
    }
    return false;
}

/**
 * sort filelist.
 */
void PatchFile::sortAsc(FileList* fileList) {
    fileList->files.sort(__fileComp);
}

/**
 * dump FileList.
 */
void PatchFile::dump(const FileList& fileList) {
    for (const auto& entry : fileList.files) {
        if (entry.isDirectory) {
            std::cout << "D ";
        } else {
            std::cout << "F ";
        }

        if (entry.isAdd) {
            std::cout << "+ ";
        } else if (entry.isRemove) {
            std::cout << "- ";
        } else if (entry.isModify) {
            std::cout << "M ";
        }
        std::cout << entry.name << std::endl;
    }
}

/**
 * calc diff between old-filelist and new-filelist
 */
FileList PatchFile::calcDiff(
    const FileList& oldList, const FileList& newList) {
    FileList fileList;

    auto oldItr = oldList.files.begin();
    auto newItr = newList.files.begin();

    while (oldItr != oldList.files.end() && newItr != newList.files.end()) {
        if (oldItr->name == newItr->name) {
            // file exists both.
            if (oldItr->isDirectory) {
                // old is directory.
                if (newItr->isDirectory) {
                    // new is directory.
                    // skip.
                    oldItr++;
                    newItr++;
                } else {
                    // new is file.
                    // old directory is removed.
                    File fileOld = *oldItr;
                    fileOld.isRemove = true;
                    // new file is added.
                    File fileNew = *newItr;
                    fileNew.isAdd = true;
                    fileList.files.push_back(fileOld);
                    fileList.files.push_back(fileNew);

                    oldItr++;
                    newItr++;
                }
            } else {
                // old is file.
                if (newItr->isDirectory) {
                    // new is directory.
                    // old file is removed.
                    File fileOld = *oldItr;
                    fileOld.isRemove = true;
                    // new directory is added.
                    File fileNew = *newItr;
                    fileNew.isAdd = true;
                    fileList.files.push_back(fileOld);
                    fileList.files.push_back(fileNew);

                    oldItr++;
                    newItr++;
                } else {
                    // new is file.
                    // check if two files are same.
                    if (isFileEqual(*oldItr, *newItr)) {
                        // two file are same.
                        // skip.
                        oldItr++;
                        newItr++;
                    } else {
                        // two files are different.
                        File fileNew = *newItr;
                        fileNew.isModify = true;
                        fileList.files.push_back(fileNew);

                        oldItr++;
                        newItr++;
                    }
                }
            }
        } else if (oldItr->name < newItr->name) {
            // old file/directory is removed.
            File fileOld = *oldItr;
            fileOld.isRemove = true;
            fileList.files.push_back(fileOld);
            oldItr++;
        } else {
            // new file/directory is added.
            File fileNew = *newItr;
            fileNew.isAdd = true;
            fileList.files.push_back(fileNew);
            newItr++;
        }
    }

    if (oldItr == oldList.files.end() && newItr != newList.files.end()) {
        // add rest new files.
        for (; newItr != newList.files.end(); newItr++) {
            File fileNew = *newItr;
            fileNew.isAdd = true;
            fileList.files.push_back(fileNew);
        }
    } else if (oldItr != oldList.files.end() && newItr == newList.files.end()) {
        // remove rest old files.
        for (; oldItr != oldList.files.end(); oldItr++) {
            File fileOld = *oldItr;
            fileOld.isRemove = true;
            fileList.files.push_back(fileOld);
        }
    }

    return fileList;
}

/**
 * check if two files are same.
 */
bool PatchFile::isFileEqual(const File& file1, const File& file2) {
    FILE *fp1, *fp2;
    const int BUFFER_SIZE = 1024;
    char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE];

    fp1 = fopen(file1.name.c_str(), "rb");
    if (fp1 == nullptr) {
        return false;
    }
    fp2 = fopen(file1.name.c_str(), "rb");
    if (fp2 == nullptr) {
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
        isContinue = (readCount1 == 1024) && (readCount2 == 1024);
        ptr1 = buf1;
        ptr2 = buf2;
        while (readCount1-- > 0) {
            if (readCount2-- == 0) {
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

void PatchFile::writeFileInfo(FILE* fp, const FileList& fileList) {
    uint32_t numEntries = fileList.files.size();
    fwrite(&numEntries, sizeof(uint32_t), 1, fp);

    for (const auto& entry : fileList.files) {
        // filename
        uint16_t length = entry.name.length();
        fwrite(&length, sizeof(uint16_t), 1, fp);
        fwrite(entry.name.c_str(), sizeof(char), length, fp);

        // flags
        uint16_t flags = encodeFlags(entry);
        fwrite(&flags, sizeof(uint16_t), 1, fp);

        // filePos (rewrite after putting data.)
        uint64_t filePos = 0;
        fwrite(&filePos, sizeof(uint64_t), 1, fp);

        // fileSize (rewrite after putting data.)
        uint64_t fileSize = 0;
        fwrite(&fileSize, sizeof(uint64_t), 1, fp);
    }
}

uint16_t PatchFile::encodeFlags(const File& file) {
    return
        (file.isDirectory ? 1 : 0)
        | (file.isAdd ? 1 : 0) << 1
        | (file.isRemove ? 1 : 0) << 2
        | (file.isModify ? 1 : 0) << 3;
}

void PatchFile::decodeFlags(uint16_t flags, File* file) {
    file->isDirectory = flags & 0x01;
    file->isAdd = (flags & 0x02) >> 1;
    file->isRemove = (flags & 0x04) >> 2;
    file->isModify = (flags & 0x08) >> 3;
}
