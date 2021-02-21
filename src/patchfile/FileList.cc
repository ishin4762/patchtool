// Copyright (C) 2020 ISHIN.
#if WINDOWS
#include <fileapi.h>
#endif

#include <iostream>
#include "FileList.h"

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

std::regex reHiddenFiles("^\\..*");

/**
 * search and enumerate files and dirs.
 */
void FileList::search(
    const std::string& path,
        bool isHiddenSearch,
        bool isCheckIgnore,
        const std::regex& reIgnorePattern) {
    for (const auto& entry : fs::directory_iterator(TO_PATH(path))) {
        // check ignore pattern
        std::string filename = TO_STR(entry.path().filename());
        if (isCheckIgnore && std::regex_match(filename, reIgnorePattern)) {
            std::cout << "skip(ignore match): " << filename << std::endl;
            continue;
        }

#if WINDOWS
        // check hidden pattern
        uint32_t attr = GetFileAttributesA(TO_STR(entry.path()).c_str());
        if (!isHiddenSearch && (attr & 0x2)) {
            std::cout << "skip(hidden file): " << filename << std::endl;
            continue;
        }
#else
        // check hidden pattern
        if (!isHiddenSearch &&
            std::regex_match(filename, reHiddenFiles)) {
            std::cout << "skip(hidden file): " << filename << std::endl;
            continue;
        }
#endif

#ifdef FS_EXPERIMENTAL
        if (fs::is_regular_file(entry) || fs::is_directory(entry)) {
            File file;
            fs::path basePath(rootDir);
            // calc relative path from rootDir
            std::string relativePath =
                (TO_STR(entry.path())).replace(0, TO_STR(basePath).size(), "");
            if (relativePath.at(0) == '\\') {
                relativePath.replace(0, 1, "");
            }
            file.name = relativePath;
            file.fullFilename = TO_STR(entry.path());
            file.isDirectory =
                entry.status().type() == fs::file_type::directory;
            files.push_back(file);

            if (fs::is_directory(entry)) {
                search(
                    TO_STR(entry.path()),
                    isHiddenSearch,
                    isCheckIgnore,
                    reIgnorePattern);
            }
        } else {
            std::cout << "skip(unknown status): " << filename << std::endl;
        }
#else
        if (entry.is_regular_file() || fs::is_directory(entry)) {
            File file;
            fs::path basePath(rootDir);
            file.name = TO_STR(fs::relative(entry.path(), basePath));
            file.fullFilename = TO_STR(entry.path());
            file.isDirectory =
                entry.status().type() == fs::file_type::directory;
            files.push_back(file);

            if (entry.is_directory()) {
                search(
                    TO_STR(entry.path()),
                    isHiddenSearch,
                    isCheckIgnore,
                    reIgnorePattern);
            }
        } else {
            std::cout << "skip(unknown status): " << filename << std::endl;
        }
#endif
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
                    fileNew.fileNewSize =
                        fs::file_size(TO_PATH(fileNew.fullFilename));
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
                            fs::file_size(TO_PATH(fileNew.fullFilename));
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
            if (!newItr->isDirectory) {
                fileNew.fileNewSize =
                    fs::file_size(TO_PATH(fileNew.fullFilename));
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
            fileNew.fileNewSize = fs::file_size(TO_PATH(fileNew.fullFilename));
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

    fp1 = fopen(file1.fullFilename.c_str(), "rb");
    if (fp1 == nullptr) {
        std::cerr << "cannot open " << file1.fullFilename << std::endl;
        return false;
    }
    fp2 = fopen(file2.fullFilename.c_str(), "rb");
    if (fp2 == nullptr) {
        std::cerr << "cannot open " << file2.fullFilename << std::endl;
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

#ifdef WINDOWS
#include <windows.h>

std::wstring File::charsToWchars(const std::string& in) {
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

std::string File::wcharsToChars(const std::wstring& in) {
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
