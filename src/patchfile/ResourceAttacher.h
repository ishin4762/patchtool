// Copyright (C) 2020 ISHIN.
#ifndef SRC_PATCHFILE_RESOURCEATTACHER_H_
#define SRC_PATCHFILE_RESOURCEATTACHER_H_

#include <string>

class ResourceAttacher {
 public:
    ResourceAttacher() {}
    static bool attach(
        const std::string& target,
        const std::string& out,
        const std::string& resource);
    static bool getResource(
        FILE* fp,
        uint64_t* offset);
};

#endif  // SRC_PATCHFILE_RESOURCEATTACHER_H_
