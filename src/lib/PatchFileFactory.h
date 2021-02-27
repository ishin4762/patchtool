// Copyright (C) 2020 ISHIN.
#ifndef SRC_LIB_PATCHFILEFACTORY_H_
#define SRC_LIB_PATCHFILEFACTORY_H_

#include <string>

namespace patchtool {

class PatchFile;

class PatchFileFactory {
 public:
    /**
     * factory method.
     */
    PatchFile* create(const std::string& compressMode);

    /**
     * factory method from file.
     */
    PatchFile* fromFile(const std::string& file);

    /**
     * factory method from file pointer.
     */
    PatchFile* fromFilePointer(FILE* fp, uint64_t offset);
};

}  // namespace patchtool

#endif  // SRC_LIB_PATCHFILEFACTORY_H_
