// Copyright (C) 2020 ISHIN.
#include <iostream>

#include "FileList.h"

namespace patchtool {

/**
 * internal compare function.
 */
static bool __fileComp(const File& v1, const File& v2) {
    if (v1.name < v2.name) {
        return true;
    }
    return false;
}

void FileList::sortAsc() {
    files.sort(__fileComp);
}

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

void FileList::search(
    FileAccess* fileAccess,
    const std::string& path,
    bool isHiddenSearch,
    bool isCheckIgnore,
    const std::regex& reIgnorePattern) {
    for (const std::string& filename
        : fileAccess->searchDirectory(path)) {
        // check ignore pattern
        if (isCheckIgnore && std::regex_match(filename, reIgnorePattern)) {
            std::cout << "skip(ignore match): " << filename << std::endl;
            continue;
        }

        // check hidden pattern
        if (!isHiddenSearch && fileAccess->isHiddenFile(filename)) {
            std::cout << "skip(hidden file): " << filename << std::endl;
            continue;
        }

        if (fileAccess->isFile(filename) || fileAccess->isDirectory(filename)) {
            File file;
            file.name = fileAccess->getRelativePath(filename, rootDir);
            file.fullFilename = filename;
            file.isDirectory = fileAccess->isDirectory(filename);
            files.push_back(file);

            if (fileAccess->isDirectory(filename)) {
                search(
                    fileAccess,
                    filename,
                    isHiddenSearch,
                    isCheckIgnore,
                    reIgnorePattern);
            }
        } else {
            std::cout << "skip(unknown status): " << filename << std::endl;
        }
    }
}

FileList FileList::searchDiff(
    FileAccess* fileAccess,
    const std::string& oldDir,
    const std::string& newDir,
    bool isHiddenSearch,
    const std::string& ignorePattern) {

    std::regex reIgnorePattern(ignorePattern);

    FileList oldFileList, newFileList;
    oldFileList.rootDir = oldDir;
    oldFileList.search(
        fileAccess,
        oldDir,
        isHiddenSearch,
        !ignorePattern.empty(),
        reIgnorePattern);
    oldFileList.sortAsc();

    newFileList.rootDir = newDir;
    newFileList.search(
        fileAccess,
        newDir,
        isHiddenSearch,
        !ignorePattern.empty(),
        reIgnorePattern);
    newFileList.sortAsc();

    FileList diffList = calcDiff(
        fileAccess, oldFileList, newFileList);
    diffList.dump();
    return diffList;
}

FileList FileList::calcDiff(
    FileAccess* fileAccess,
    const FileList& oldList,
    const FileList& newList) {
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
                    fileOld.fileNewSize = 0;
                    fileOld.oldFilename = fileOld.fullFilename;
                    // new file is added.
                    File fileNew = *newItr;
                    fileNew.isAdd = true;
                    fileNew.newFilename = fileNew.fullFilename;
                    fileNew.fileNewSize =
                        fileAccess->getFileSize(fileNew.fullFilename);
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
                    fileOld.fileNewSize = 0;
                    fileOld.oldFilename = fileOld.fullFilename;
                    // new directory is added.
                    File fileNew = *newItr;
                    fileNew.isAdd = true;
                    fileNew.fileNewSize = 0;
                    fileList.files.push_back(fileOld);
                    fileList.files.push_back(fileNew);

                    oldItr++;
                    newItr++;
                } else {
                    // new is file.
                    // check if two files are same.
                    if (fileAccess->isFileEqual(
                        oldItr->fullFilename, newItr->fullFilename)) {
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
                            fileAccess->getFileSize(fileNew.fullFilename);
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
            fileOld.fileNewSize = 0;
            fileOld.oldFilename = fileOld.fullFilename;
            fileList.files.push_back(fileOld);
            oldItr++;
        } else {
            // new file/directory is added.
            File fileNew = *newItr;
            fileNew.isAdd = true;
            fileNew.newFilename = fileNew.fullFilename;
            if (!newItr->isDirectory) {
                fileNew.fileNewSize =
                    fileAccess->getFileSize(fileNew.fullFilename);
            } else {
                fileNew.fileNewSize = 0;
            }
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
            fileNew.fileNewSize =
                fileAccess->getFileSize(fileNew.fullFilename);
            fileList.files.push_back(fileNew);
        }
    } else if (oldItr != oldList.files.end() && newItr == newList.files.end()) {
        // remove rest old files.
        for (; oldItr != oldList.files.end(); oldItr++) {
            File fileOld = *oldItr;
            fileOld.isRemove = true;
            fileOld.fileNewSize = 0;
            fileOld.oldFilename = fileOld.fullFilename;
            fileList.files.push_back(fileOld);
        }
    }

    return fileList;
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

}  // namespace patchtool
