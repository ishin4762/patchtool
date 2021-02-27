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

#include "PatchEncoder.h"

namespace patchtool {

bool PatchEncoder::encode(
    const std::string& oldDir,
    const std::string& newDir,
    const std::string& output,
    bool isHiddenSearch,
    const std::string& ignorePattern) {

    // search directory diff.
    std::cout << "Check diff..." << std::endl;
    FileList diffList = FileList::searchDiff(
        fileAccess, oldDir, newDir, isHiddenSearch, ignorePattern);

    // write file.
    std::cout << std::endl << "Writing..." << std::endl;
    FILE* fp = fileAccess->openWriteFile(output);
    if (fp == nullptr) {
        std::cerr << "cannot create " << output << std::endl;
        return false;
    }

    create(fp, &diffList);

    // flush.
    fileAccess->closeFile(fp);

    return true;
}

void PatchEncoder::create(FILE* fp, FileList* fileList) {
    // 0) write signature.
    if (isVerbose) {
        std::cout << "Writing signature..." << std::endl;
    }
    fileAccess->write(
        fp, patchFile->getSignature(), patchFile->getSignatureSize());

    // 1) write header. (filePos & fileSize must be zero.)
    if (isVerbose) {
        std::cout << "Writing headers..." << std::endl;
    }
    writeHeader(fp, fileList, true);

    // 2) write data.
    if (isVerbose) {
        std::cout << "Writing data..." << std::endl;
    }
    writeData(fp, fileList);

    // 3) re-write header. (filePos & fileSize will be non-zero.)
    if (isVerbose) {
        std::cout << "Rewriting headers..." << std::endl;
    }
    writeHeader(fp, fileList, false);
}

void PatchEncoder::writeHeader(
    FILE* fp, FileList* fileList, bool isFirstCall) {
    // move pointer to head.
    fileAccess->seek(fp, patchFile->getSignatureSize(), SEEK_SET);

    // version info.
    fileAccess->write<uint32_t>(fp, 0xffffffff);
    fileAccess->write<uint32_t>(fp, 2);

    // meta data length. (for future use)
    fileAccess->write<uint32_t>(fp, 0);

    // file entries.
    uint32_t numEntries = fileList->files.size();
    fileAccess->write<uint32_t>(fp, numEntries);
    for (auto& entry : fileList->files) {
        writeFileInfo(fp, &entry, isFirstCall);
    }
}

void PatchEncoder::writeFileInfo(
    FILE* fp, File* file, bool isFirstCall) {
    // filename
    fileAccess->write<uint16_t>(fp, file->name.length());
    fileAccess->write(fp, file->name.c_str(), file->name.length());

    // flags
    fileAccess->write<uint16_t>(fp, file->encodeFlags());

    // filePos
    fileAccess->write<uint64_t>(fp, file->filePos);

    // fileSize
    fileAccess->write<uint64_t>(fp, file->fileSize);

    // fileNewSize
    fileAccess->write<uint64_t>(fp, file->fileNewSize);

    // checkSum
    fileAccess->write<uint32_t>(fp, file->checkSum);

    // allocate blockSizeList
    if (isFirstCall) {
        uint64_t numBlocks = file->fileNewSize / (blockSize * 1024);
        if (file->fileNewSize % (blockSize * 1024) > 0) {
            numBlocks++;
        }
        file->numBlocks = numBlocks;
        file->oldBlockSizeList.resize(numBlocks);
        file->newBlockSizeList.resize(numBlocks);
        file->diffBlockSizeList.resize(numBlocks);
    }

    // blockSize
    fileAccess->write<uint64_t>(fp, file->numBlocks);
    for (auto value : file->oldBlockSizeList) {
        fileAccess->write<uint32_t>(fp, value);
    }
    for (auto value : file->newBlockSizeList) {
        fileAccess->write<uint32_t>(fp, value);
    }
    for (auto value : file->diffBlockSizeList) {
        fileAccess->write<uint32_t>(fp, value);
    }

    if (isVerbose) {
        std::cout << "Name: " << file->name << std::endl;
        std::cout << "Flags: " << file->encodeFlags() << std::endl;
        std::cout << "FilePos: " << file->filePos << std::endl;
        std::cout << "FileSize: " << file->fileSize << std::endl;
        std::cout << "FileNewSize: " << file->fileNewSize << std::endl;
        std::cout << "Checksum: " << file->checkSum << std::endl;
        std::cout << "NumBlocks: " << file->numBlocks
            << std::endl << std::endl;
    }
}

void PatchEncoder::writeData(FILE* fp, FileList* fileList) {
    for (auto& entry : fileList->files) {
        writeFile(fp, &entry);
    }
}

void PatchEncoder::writeFile(FILE* fp, File* file) {
    if (file->isDirectory) {
        return;
    }

    std::cout << file->name << " ...";

    if (file->isAdd) {
        writeAdditionalFile(fp, file);
    } else if (file->isModify) {
        writeUpdateFile(fp, file);
    }

    if (file->isModify || file->isRemove) {
        file->checkSum = fileAccess->calcCheckSum(file->oldFilename);
    }

    if (file->fileNewSize > 0) {
        int64_t percent = file->fileSize * 100 /file->fileNewSize;
        std::cout << file->fileSize << "/" << file->fileNewSize
            << " bytes (" << percent << "%)" << std::endl;
    } else {
        std::cout << file->fileSize << "/" << file->fileNewSize
            << " bytes (---%)" << std::endl;
    }
}

void PatchEncoder::writeAdditionalFile(FILE* fp, File* file) {
    uint64_t oldFileSize = 0;
    uint64_t newFileSize;
    uint32_t blockSizeBytes = blockSize * 1024;
    uint8_t* bufOld = new uint8_t[oldFileSize+1];
    uint8_t* bufNew = new uint8_t[blockSizeBytes+1];

    // set data begin pos.
    file->filePos = fileAccess->tell(fp);

    // write diff by chunk.
    FILE* newFp = fileAccess->openReadFile(file->newFilename);
    if (newFp) {
        uint64_t prevPos = fileAccess->tell(fp);
        for (uint64_t i = 0; i < file->numBlocks; i++) {
            // read new data.
            if (!fileAccess->readBlock(
                newFp, bufNew, &newFileSize, blockSizeBytes)) {
                break;
            }

            // write diff.
            patchFile->openWriter(fp);
            bsdiff(
                bufOld, oldFileSize, bufNew, newFileSize,
                patchFile->getWriteStream());
            patchFile->closeWriter();

            // calc write size.
            uint64_t nowPos = fileAccess->tell(fp);
            uint64_t delta = nowPos - prevPos;
            prevPos = nowPos;

            // set block size.
            file->oldBlockSizeList[i] = 0;
            file->newBlockSizeList[i] = newFileSize;
            file->diffBlockSizeList[i] = delta;
        }
        fileAccess->closeFile(newFp);
    }

    delete[] bufOld;
    delete[] bufNew;

    // set total write size.
    file->fileSize = fileAccess->tell(fp) - file->filePos;
}

void PatchEncoder::writeUpdateFile(FILE* fp, File* file) {
    uint64_t oldFileSize;
    uint64_t newFileSize;
    uint32_t blockSizeBytes = blockSize * 1024;
    uint8_t* bufOld = new uint8_t[blockSizeBytes+1];
    uint8_t* bufNew = new uint8_t[blockSizeBytes+1];

    // set data begin pos.
    file->filePos = fileAccess->tell(fp);

    // write diff by chunk.
    FILE* oldFp = fileAccess->openReadFile(file->oldFilename);
    FILE* newFp = fileAccess->openReadFile(file->newFilename);
    if (oldFp && newFp) {
        uint64_t prevPos = fileAccess->tell(fp);
        for (uint64_t i = 0; i < file->numBlocks; i++) {
            // read old data.
            if (!fileAccess->readBlock(
                oldFp, bufOld, &oldFileSize, blockSizeBytes)) {
                break;
            }

            // read new data.
            if (!fileAccess->readBlock(
                newFp, bufNew, &newFileSize, blockSizeBytes)) {
                break;
            }

            // write diff.
            patchFile->openWriter(fp);
            bsdiff(
                bufOld, oldFileSize, bufNew, newFileSize,
                patchFile->getWriteStream());
            patchFile->closeWriter();

            // calc write size.
            uint64_t nowPos = fileAccess->tell(fp);
            uint64_t delta = nowPos - prevPos;
            prevPos = nowPos;

            // set block size.
            file->oldBlockSizeList[i] = oldFileSize;
            file->newBlockSizeList[i] = newFileSize;
            file->diffBlockSizeList[i] = delta;
        }
    }
    fileAccess->closeFile(oldFp);
    fileAccess->closeFile(newFp);

    delete[] bufOld;
    delete[] bufNew;

    // set total write size.
    file->fileSize = fileAccess->tell(fp) - file->filePos;
}

}  // namespace patchtool
