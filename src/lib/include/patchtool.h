// Copyright(C) 2020 ISHIN.
#ifndef SRC_LIB_INCLUDE_PATCHTOOL_H_
#define SRC_LIB_INCLUDE_PATCHTOOL_H_

#include <string>

namespace patchtool {

class Patch {
 public:
    Patch() {}
    bool encode(
        const std::string& oldDir,
        const std::string& newDir,
        const std::string& output,
        const std::string& mode,
        bool isHiddenSearch,
        const std::string& ignorePattern,
        uint32_t blockSize,
        bool isVerbose);
    bool decode(
        const std::string& targetDir,
        const std::string& input,
        bool isVerbose);
    bool decode(
        const std::string& targetDir,
        FILE *fp,
        const uint64_t offset,
        bool isVerbose);
    bool attach(
        const std::string& target,
        const std::string& out,
        const std::string& resource);
    bool getResource(
        FILE* fp,
        uint64_t* offset);
};

}  // namespace patchtool

#endif  // SRC_LIB_INCLUDE_PATCHTOOL_H_
