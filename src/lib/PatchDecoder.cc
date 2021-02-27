// Copyright (C) 2020 ISHIN.
extern "C" {
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #include <fcntl.h>
}

#include <cstring>
#include <iostream>
#include <random>
#include <ctime>
#include <iomanip>

extern "C" {
  #include "external_lib/bsdiff/bsdiff.h"
  #include "external_lib/bsdiff/bspatch.h"
}

#include "PatchDecoder.h"

namespace patchtool {

bool PatchDecoder::decode(
    const std::string& targetDir) {

    // generate suffix for patch applying
    suffix = generateSuffix();

    // 0) check signature.
    if (isVerbose) {
        std::cout << "Check signature..." << std::endl;
    }
    if (!checkSignature()) {
        std::cerr << "invalid signature." << std::endl;
        return false;
    }

    // 1) read header.
    if (isVerbose) {
        std::cout << "Read headers..." << std::endl;
    }
    FileList fileList;
    fileList.rootDir = targetDir;
    if (!readHeader(&fileList)) {
        std::cerr << "invalid header." << std::endl;
        return false;
    }

    // 2) validate file existence & checksum.
    if (isVerbose) {
        std::cout << "Validate files..." << std::endl;
    }
    if (!validateFiles(fileList)) {
        return false;
    }

    // 3) apply.
    std::cout << "Applying..." << std::endl;
    bool isSucceeded = applyFiles(fileList);

    // 4) cleanup.
    if (isSucceeded) {
        std::cout << "Cleanup..." << std::endl;
    } else {
        std::cout << "Rollback..." << std::endl;
    }
    if (!cleanupFiles(targetDir, isSucceeded)) {
        return false;
    }

    return isSucceeded;
}

bool PatchDecoder::checkSignature() {
    uint8_t signature[16] = { 0 };

    // seek to data begin.
    fileAccess->seek(fp, offset, SEEK_SET);

    // read signature.
    if (fileAccess->read(fp, signature, 16) < 16) {
        std::cerr << "invalid header." << std::endl;
        return false;
    }
    return true;
}

bool PatchDecoder::readHeader(FileList* fileList) {
    // seek to header.
    fileAccess->seek(fp, offset + 16, SEEK_SET);

    // # of entries. (ver = 1)
    // 0xffffffff (ver >= 2)
    uint32_t numEntries;
    if (fileAccess->read<uint32_t>(fp, &numEntries) < 1) {
        return false;
    }

    // version and # of entries(ver. >= 2)
    uint32_t version = 1;
    if (numEntries == 0xffffffff) {
        // version.
        if (fileAccess->read<uint32_t>(fp, &version) < 1) {
            return false;
        }

        // meta data length. (for future use)
        uint32_t metaLength = 0;
        if (fileAccess->read<uint32_t>(fp, &metaLength) < 1) {
            return false;
        }
        // skip meta data.
        fileAccess->seek(fp, metaLength, SEEK_CUR);

        // get # of entries.
        if (fileAccess->read<uint32_t>(fp, &numEntries) < 1) {
            return false;
        }
    }

    // file entries.
    for (uint32_t i=0; i < numEntries; i++) {
        File file;
        if (!readFileInfo(&file, version)) {
            return false;
        }
        fileList->files.push_back(file);
    }

    return true;
}

bool PatchDecoder::readFileInfo(File* file, int version) {
    // filename.
    std::string name;
    if (!fileAccess->readString(fp, &name)) {
        return false;
    }
    file->name = name;

    // flags.
    uint16_t flags;
    if (fileAccess->read<uint16_t>(fp, &flags) < 1) {
        return false;
    }
    file->decodeFlags(flags);

    // filePos
    uint64_t filePos;
    if (fileAccess->read<uint64_t>(fp, &filePos) < 1) {
        return false;
    }
    file->filePos = filePos;

    // fileSize
    uint64_t fileSize;
    if (fileAccess->read<uint64_t>(fp, &fileSize) < 1) {
        return false;
    }
    file->fileSize = fileSize;

    // fileNewSize
    uint64_t fileNewSize;
    if (fileAccess->read<uint64_t>(fp, &fileNewSize) < 1) {
        return false;
    }
    file->fileNewSize = fileNewSize;

    // checkSum
    uint32_t checkSum;
    if (fileAccess->read<uint32_t>(fp, &checkSum) < 1) {
        return false;
    }
    file->checkSum = checkSum;

    // block size list.
    if (version >= 2) {
        // numBlocks
        uint64_t numBlocks;
        if (fileAccess->read<uint64_t>(fp, &numBlocks) < 1) {
            return false;
        }
        file->numBlocks = numBlocks;

        // allocate block size list.
        file->oldBlockSizeList.resize(numBlocks);
        file->newBlockSizeList.resize(numBlocks);
        file->diffBlockSizeList.resize(numBlocks);

        for (uint64_t i = 0; i < numBlocks; i++) {
            uint32_t val;
            if (fileAccess->read<uint32_t>(fp, &val) < 1) {
                return false;
            }
            file->oldBlockSizeList[i] = val;
        }

        for (uint64_t i = 0; i < numBlocks; i++) {
            uint32_t val;
            if (fileAccess->read<uint32_t>(fp, &val) < 1) {
                return false;
            }
            file->newBlockSizeList[i] = val;
        }

        for (uint64_t i = 0; i < numBlocks; i++) {
            uint32_t val;
            if (fileAccess->read<uint32_t>(fp, &val) < 1) {
                return false;
            }
            file->diffBlockSizeList[i] = val;
        }
    } else {
        file->numBlocks = 1;
        file->oldBlockSizeList.resize(1);
        file->newBlockSizeList.resize(1);
        file->diffBlockSizeList.resize(1);
        file->oldBlockSizeList[0] = file->fileSize;
        file->newBlockSizeList[0] = file->fileNewSize;
        file->diffBlockSizeList[0] = file->fileNewSize;
    }

    if (isVerbose) {
        std::cout << "Name: " << file->name << std::endl;
        std::cout << "Flags: " << file->encodeFlags() << std::endl;
        std::cout << "FilePos: " << file->filePos << std::endl;
        std::cout << "FileSize: " << file->fileSize << std::endl;
        std::cout << "FileNewSize: " << file->fileNewSize << std::endl;
        std::cout << "Checksum: " << file->checkSum << std::endl;
        std::cout << "NumBlocks: " <<
            file->numBlocks << std::endl << std::endl;
    }

    return true;
}

bool PatchDecoder::validateFiles(const FileList& fileList) {
    for (const File& file : fileList.files) {
        if (!validateFile(fileList.rootDir, file)) {
            return false;
        }
    }
    return true;
}

bool PatchDecoder::validateFile(
    const std::string& baseDir, const File& file) {
    std::string filePath =
        baseDir + "/" + file.name;

    // old file is removed or modified.
    if (file.isRemove || file.isModify) {
        if (!fileAccess->fileExists(filePath)) {
            std::cerr << (file.isDirectory ? "directory" : "file")
                << " " << file.name << " is not found." << std::endl;
            return false;
        }

        // check checkSum.
        if (!file.isDirectory) {
            uint32_t checkSum = fileAccess->calcCheckSum(filePath);

            if (isVerbose) {
                std::cout << "Name: " << file.name << std::endl;
                std::cout << "FileCheckSum: " << checkSum << std::endl;
                std::cout << "CompareCheckSum: "
                    << file.checkSum << std::endl;
            }

            if (checkSum != file.checkSum) {
                std::cerr << "checksum " << file.name
                    << " is not match." << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool PatchDecoder::applyFiles(const FileList& fileList) {
    // map of renamed directories.
    std::unordered_map<std::string, std::string> nameMap;

    // apply for each file.
    for (const File& file : fileList.files) {
        if (!applyFile(fileList.rootDir, file, &nameMap)) {
            return false;
        }
    }
    return true;
}

bool PatchDecoder::applyFile(
    const std::string& baseDir,
    const File& file,
    std::unordered_map<std::string, std::string>* nameMap) {

    std::cout << file.name << std::endl;

    if (file.isDirectory) {
        if (file.isAdd) {
            // add directory
            std::string afterPath = applyRename(
                nameMap, baseDir, file.name, suffix + ".add", true);
            if (!fileAccess->createDirectory(afterPath)) {
                return false;
            }
        } else if (file.isRemove) {
            // remove directory.
            std::string beforePath = applyRename(
                nameMap, baseDir, file.name, "", false);
            std::string afterPath = applyRename(
                nameMap, baseDir, file.name, suffix + ".remove", true);
            if (!fileAccess->renameFile(beforePath, afterPath)) {
                return false;
            }
        }
    } else {
        if (file.isAdd) {
            // add file.
            std::string afterPath = applyRename(
                nameMap, baseDir, file.name, suffix + ".add", false);
            if (!generateFile(afterPath, file)) {
                return false;
            }
        } else if (file.isRemove) {
            // remove file.
            std::string beforePath = applyRename(
                nameMap, baseDir, file.name, "", false);
            std::string afterPath = applyRename(
                nameMap, baseDir, file.name, suffix + ".remove", false);
            if (!fileAccess->renameFile(beforePath, afterPath)) {
                return false;
            }
        } else if (file.isModify) {
            // modify file.
            std::string beforePath = applyRename(
                nameMap, baseDir, file.name, "", false);
            std::string afterPath = applyRename(
                nameMap, baseDir, file.name, suffix + ".modify", false);
            // copy file
            if (!fileAccess->copyFile(beforePath, afterPath)) {
                std::cerr << "cannot modify " << file.name << std::endl;
                return false;
            }

            // apply patch for copied file.
            if (!updateFile(afterPath, file)) {
                std::cerr << "cannot modify " << file.name << std::endl;
                return false;
            }
        }
    }

    return true;
}

bool PatchDecoder::cleanupFiles(
    const std::string& baseDir, bool isSucceeded) {

    for (const std::string& path
        : fileAccess->searchDirectory(baseDir)) {
        if (fileAccess->isDirectory(path)) {
            // call recursively.
            if (!cleanupFiles(path, isSucceeded)) {
                return false;
            }
        }
        if (!cleanupFile(path, isSucceeded)) {
            return false;
        }
    }
    return true;
}

bool PatchDecoder::cleanupFile(
    const std::string& path, bool isSucceeded) {

    if (stringEndsWith(path, suffix + ".add")) {
        if (isSucceeded) {
            // if succeeded, get rid of filename suffix.
            std::string afterPath =
                trimSuffix(path, suffix + ".add");
            if (!fileAccess->renameFile(path, afterPath)) {
                return false;
            }
        } else {
            // if failed, remove file.
            if (!fileAccess->removeFile(path)) {
                if (!fileAccess->isDirectory(path)) {
                    return false;
                }
            }
        }
    } else if (stringEndsWith(path, suffix + ".remove")) {
        if (isSucceeded) {
            // if succeeded, remove file.
            if (!fileAccess->removeFile(path)) {
                if (!fileAccess->isDirectory(path)) {
                    return false;
                }
            }
        } else {
            // if failed, get rid of filename suffix.
            std::string afterPath =
                trimSuffix(path, suffix + ".remove");
            if (!fileAccess->renameFile(path, afterPath)) {
                return false;
            }
        }
    } else if (stringEndsWith(path, suffix + ".modify")) {
        if (isSucceeded) {
            // if succeeded,
            std::string afterPath =
                trimSuffix(path, suffix + ".modify");
            std::string tempPath =
                path + suffix + ".bak";

            // 1) rename original filename to backup.
            if (!fileAccess->renameFile(afterPath, tempPath)) {
                return false;
            }

            // 2) get rid of suffix.
            if (!fileAccess->renameFile(path, afterPath)) {
                return false;
            }

            // 3) remove backup file.
            // continue even if error.
            fileAccess->removeFile(tempPath);
        } else {
            // if failed, remove file.
            if (!fileAccess->removeFile(path)) {
                if (!fileAccess->isDirectory(path)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool PatchDecoder::generateFile(
    const std::string& path, const File& file) {
    // allocate file buffer.
    uint8_t* bufNew = new uint8_t[file.newBlockSizeList[0] + 1];
    uint8_t* bufOld = new uint8_t[1];
    memset(bufNew, 0, file.newBlockSizeList[0] + 1);

    // seek to data begin pos.
    fileAccess->seek(fp, file.filePos + offset, SEEK_SET);

    bool ret = true;
    FILE* newFp = fileAccess->openWriteFile(path);
    for (size_t i = 0; i < file.numBlocks; i++) {
        uint64_t newFileSize = file.newBlockSizeList[i];
        uint64_t oldFileSize = 0;

        // generate tempfile from diff file's block.
        std::string tmpFileName = path + ".tmp";
        if (!fileAccess->createTempFile(
            tmpFileName, fp, file.diffBlockSizeList[i])) {
            ret = false;
            break;
        }

        FILE* tmpFp = fileAccess->openReadFile(tmpFileName);
        if (tmpFp == nullptr) {
            fileAccess->removeFile(tmpFileName);
            ret = false;
            break;
        }

        // apply patch
        patchFile->openReader(tmpFp);
        bspatch(
            bufOld, oldFileSize, bufNew, newFileSize,
            patchFile->getReadStream());
        patchFile->closeReader();

        // remove tempfile
        fileAccess->closeFile(tmpFp);
        fileAccess->removeFile(tmpFileName);

        // write file.
        if (!fileAccess->writeBlock(newFp, bufNew, newFileSize)) {
            ret = false;
            break;
        }
    }

    fileAccess->closeFile(newFp);
    delete[] bufOld;
    delete[] bufNew;

    return ret;
}

bool PatchDecoder::updateFile(
    const std::string& path, const File& file) {
    // allocate file buffer.
    uint8_t* bufNew = new uint8_t[file.newBlockSizeList[0] + 1];
    uint8_t* bufOld = new uint8_t[file.oldBlockSizeList[0] + 1];
    memset(bufNew, 0, file.newBlockSizeList[0] + 1);

    // seek to data begin pos.
    fileAccess->seek(fp, file.filePos + offset, SEEK_SET);

    bool ret = true;
    FILE* newFp = fileAccess->openWriteFile(path + ".mod");
    FILE* oldFp = fileAccess->openReadFile(path);
    for (size_t i = 0; i < file.numBlocks; i++) {
        uint64_t newFileSize = file.newBlockSizeList[i];
        uint64_t oldFileSize;

        // read old file's block.
        if (!fileAccess->readBlock(
            oldFp, bufOld, &oldFileSize, file.oldBlockSizeList[i])) {
            ret = false;
            break;
        }

        // generate tempfile from diff file's block.
        std::string tmpFileName = path + ".tmp";
        if (!fileAccess->createTempFile(
            tmpFileName, fp, file.diffBlockSizeList[i])) {
            ret = false;
            break;
        }

        FILE* tmpFp = fileAccess->openReadFile(tmpFileName);
        if (tmpFp == nullptr) {
            fileAccess->removeFile(tmpFileName);
            ret = false;
            break;
        }

        // apply patch
        patchFile->openReader(tmpFp);
        bspatch(
            bufOld, oldFileSize, bufNew, newFileSize,
            patchFile->getReadStream());
        patchFile->closeReader();

        // remove tempfile
        fileAccess->closeFile(tmpFp);
        fileAccess->removeFile(tmpFileName);

        // write file.
        if (!fileAccess->writeBlock(newFp, bufNew, newFileSize)) {
            ret = false;
            break;
        }
    }

    fileAccess->closeFile(oldFp);
    fileAccess->closeFile(newFp);
    delete[] bufOld;
    delete[] bufNew;

    fileAccess->removeFile(path);
    fileAccess->renameFile(path + ".mod", path);

    return ret;
}

const std::string PatchDecoder::generateSuffix() {
    auto time = std::time(nullptr);
    std::random_device rnd;
    std::stringstream ss;
    ss << std::string(".")
        << std::put_time(std::gmtime(&time), "%Y%m%d%H%M%S")
        << "." << std::to_string(rnd());
    return ss.str();
}

bool PatchDecoder::stringEndsWith(
    const std::string& str, const std::string& suffix) {
    if (str.size() < suffix.size()) {
        return false;
    }
    return std::equal(std::rbegin(suffix), std::rend(suffix), std::rbegin(str));
}

const std::string PatchDecoder::trimSuffix(
    const std::string& str, const std::string& suffix) {
    if (!stringEndsWith(str, suffix)) {
        return str;
    }
    return str.substr(0, str.find(suffix, str.size() - suffix.size()));
}

const std::string PatchDecoder::applyRename(
    std::unordered_map<std::string, std::string>* nameMap,
    const std::string& baseDir,
    const std::string& filename,
    const std::string& suffix,
    bool isAdd) {

    int hitLength = 0;
    std::string output = filename + suffix;
    for (auto& kv : *nameMap) {
        if (filename.find(kv.first) == 0) {
            if (hitLength < kv.first.length()) {
                hitLength = kv.first.length();
                output = kv.second +
                    filename.substr(kv.first.length()) + suffix;
            }
        }
    }

    if (isAdd) {
        (*nameMap)[filename] = filename + suffix;
    }
    return baseDir + "/" + output;
}

}  // namespace patchtool
