// Copyright (C) 2020 ISHIN.
#include <iostream>
#include <filesystem>
#include "FileList.h"

namespace fs = std::filesystem;

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
void FileList::sortAsc() {
    files.sort(__fileComp);
}

/**
 * dump FileList.
 */
void FileList::dump() {
    for (const auto& entry : files) {
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
 * search and enumerate files and dirs.
 */
void FileList::search(const std::string& path) {
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_directory()) {
            search(entry.path().generic_string());
        } else if (entry.is_regular_file()) {
            File file;
            fs::path path = entry.path();
            fs::path basePath(rootDir);
            file.name = fs::relative(path, basePath).generic_string();
            file.fullFilename = entry.path().generic_string();
            file.isDirectory =
                entry.status().type() == fs::file_type::directory;
            files.push_back(file);
        }
    }
}

/**
 * calc diff between old-filelist and new-filelist
 */
FileList FileList::calcDiff(
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
                    fileNew.newFilename = fileNew.fullFilename;
                    fileNew.fileNewSize = fs::file_size(fileNew.fullFilename);
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
                    if (File::isEqual(*oldItr, *newItr)) {
                        // two file are same.
                        // skip.
                        oldItr++;
                        newItr++;
                    } else {
                        // two files are different.
                        File fileNew = *newItr;
                        fileNew.isModify = true;
                        fileNew.newFilename = fileNew.fullFilename;
                        fileNew.oldFilename = oldItr->fullFilename;
                        fileNew.fileNewSize =
                            fs::file_size(fileNew.fullFilename);
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
            fileNew.newFilename = fileNew.fullFilename;
            fileNew.fileNewSize = fs::file_size(fileNew.fullFilename);
            fileList.files.push_back(fileNew);
            newItr++;
        }
    }

    if (oldItr == oldList.files.end() && newItr != newList.files.end()) {
        // add rest new files.
        for (; newItr != newList.files.end(); newItr++) {
            File fileNew = *newItr;
            fileNew.isAdd = true;
            fileNew.newFilename = fileNew.fullFilename;
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
bool File::isEqual(const File& file1, const File& file2) {
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
        isContinue = (readCount1 == BUFFER_SIZE) && (readCount2 == BUFFER_SIZE);
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

/**
 * encode file type & modify type.
 */
uint16_t File::encodeFlags() {
    return
        (isDirectory ? 1 : 0)
        | (isAdd ? 1 : 0) << 1
        | (isRemove ? 1 : 0) << 2
        | (isModify ? 1 : 0) << 3;
}

/**
 * decode file type & modify type.
 */
void File::decodeFlags(uint16_t flags) {
    isDirectory = flags & 0x01;
    isAdd = (flags & 0x02) >> 1;
    isRemove = (flags & 0x04) >> 2;
    isModify = (flags & 0x08) >> 3;
}
