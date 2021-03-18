#pragma once
#include "type.h"
#include <sys/stat.h>
#include <stdio.h>
#include <mutex>
#include <string.h>
#include <assert.h>

class File {
    private:
        enum __FileFunc : char {
            __FuncRead,
            __FuncWrite
        };

    public:
        int64 start;  // beginning index of the bytes representation of the torrent
        int64 end;  // end index of the bytes representation of the torrent
        FILE* fp;  // file for writing blocks
        std::mutex mtx;  // for locking the write code before writing a block to it

        void __syncHelper(__FileFunc type, char *buf, int32 &blockSize, int64 &offset);
        void init(char filePath[], int64 start, int64 end);
        void write(char *block, int32 blockSize, int64 offset);  // implement locking resouce too
        int32 read(char *buf, int32 blockSize, int64 offset);
        bool exists(char path[]);
        int mkpath(char filePath[], mode_t mode=0700);
        void closeFile();  // call this once file is completely downloaded
        ~File();  // close file
};

namespace FSpace {
    int32 __BSearch(File* files, int32 n, int64 x);
};