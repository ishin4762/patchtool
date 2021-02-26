// Copyright (C) 2020 ISHIN.
#ifndef SRC_LIB_BZIP2PATCHFILE_H_
#define SRC_LIB_BZIP2PATCHFILE_H_

#include <string>

#include "PatchFile.h"

namespace patchtool {

class BZip2PatchFile : public PatchFile {
 public:
    BZip2PatchFile();

 protected:
    bool openWriter(FILE* fp);
    bool closeWriter();
    bool openReader(FILE* fp);
    bool closeReader();
    const char* getSignature() {
       return signature;
    }
    int getSignatureSize() {
       return sizeof(signature);
    }
    bsdiff_stream* getWriteStream() {
       return &writeStream;
    }
    bspatch_stream* getReadStream() {
       return &readStream;
    }

 private:
    bsdiff_stream writeStream;
    bspatch_stream readStream;
    const char signature[16] = "BZIP2 ver.1.01";
};

}  // namespace patchtool

#endif  // SRC_LIB_BZIP2PATCHFILE_H_
