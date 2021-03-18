#include "../include/file.h"

int32 FSpace::__BSearch(File* files, int32 n, int64 x) {
    // returns file index which includes the offset x
    int32 l, r, m;
    l = 0;
    r = n-1;
    while(l < r) {
        m = l+(r-l)/2;

        if(files[m].start > x)
            r = m-1;
        else if(files[m].end < x)
            l = m+1;
        else return m; 
    } 

    return l;
}


void File::init(char filePath[], int64 start, int64 end) {
    // get dirname and check if parent exists
    // create parent recursivelt if not exists
    mkpath(filePath);  // creates the parent directory if not exists: works for relative paths, abs paths untested
    fp = fopen(filePath, "wb");
    this->start = start;
    this->end = end;
}

bool File::exists(char path[]) {
    // https://stackoverflow.com/questions/12510874/how-can-i-check-if-a-directory-exists
    struct stat info;
    return (stat(path, &info) == 0);
}


int File::mkpath(char filePath[], mode_t mode) {
    // https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
    assert(filePath && *filePath);
    for (char* p = strchr(filePath + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = '\0';
        if (mkdir(filePath, mode) == -1) {
            if (errno != EEXIST) {
                *p = '/';
                return -1;
            }
        }
        *p = '/';
    }
    return 0;
}


void File::__syncHelper(__FileFunc type, char *buf, int32 &blockSize, int64 &offset) {
    // mutex ref: http://www.cplusplus.com/reference/mutex/mutex/
    mtx.lock();
    fseek(fp, offset, SEEK_SET);
    if(type == __FuncRead) {
        // TODO handle read errors here
        fread(buf, 1, blockSize, fp);
    }
    else {
        // TODO handle write errors here
        fwrite(buf, 1, blockSize, fp);
    }
    mtx.unlock();
}


void File::write(char *block, int blockSize, int64 offset) {
    __syncHelper(__FuncWrite, block, blockSize, offset);
}

int32 File::read(char *buf, int32 blockSize, int64 offset) {
    __syncHelper(__FuncRead, buf, blockSize, offset);
}

File::~File() {
    fclose(fp);
}