// Copyright (C) 2020 ISHIN.
#ifndef SRC_LIB_PATCHFILE_H_
#define SRC_LIB_PATCHFILE_H_

extern "C" {
  #include "external_lib/bsdiff/bsdiff.h"
  #include "external_lib/bsdiff/bspatch.h"
}

namespace patchtool {

class PatchFile {
 public:
    virtual bool openWriter(FILE* fp) = 0;
    virtual bool closeWriter() = 0;
    virtual bool openReader(FILE* fp) = 0;
    virtual bool closeReader() = 0;
    virtual const char* getSignature() = 0;
    virtual int getSignatureSize() = 0;
    virtual bsdiff_stream* getWriteStream() = 0;
    virtual bspatch_stream* getReadStream() = 0;
};

}  // namespace patchtool

#endif  // SRC_LIB_PATCHFILE_H_
