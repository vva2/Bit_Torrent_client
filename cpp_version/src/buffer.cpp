#include "../include/buffer.h"

Buffer::Buffer() {
    buffer = NULL;
    size = 0;
}

Buffer::Buffer(bint64 size) {
    alloc(size);
}

Buffer::Buffer(const char* data) {
    size = strlen(data);
    alloc(size);
    copyFrom(data, size, 0);
}

Buffer::~Buffer() {
    if(buffer) delete[] buffer;
}

void Buffer::popFront(bint64 len) {
    // removes "len" size of bytes from front of buffer 
    // slices the buffer from len
    memcpy(buffer, &buffer[len], size-len);
}

void Buffer::alloc(bint64 len) {
    buffer = new char[len];
    memset(buffer, 0, len);
    size = len;
}

Buffer::Buffer(const Buffer &from) {
    size = from.size;
    alloc(from.size);
    copyFrom(from.buffer, from.size, 0);
}

void Buffer::fill(char x) {
    memset(buffer, x, size);
}

void Buffer::writeInt8BE(bint8 data, bint64 offset) {
    buffer[offset] = data;
}

void Buffer::writeUInt8BE(buint8 data, bint64 offset) {
    buffer[offset] = data;
}

void Buffer::writeInt16BE(bint16 data, bint64 offset) {
    char* ptr = (char*) &data;
    buffer[offset] = ptr[1];
    buffer[offset+1] = ptr[0];
}

void Buffer::writeUInt16BE(buint16 data, bint64 offset) {
    char *ptr = (char*) &data;
    buffer[offset] = ptr[1];
    buffer[offset+1] = ptr[0];
}

void Buffer::writeInt32BE(bint32 data, bint64 offset) {
    char *ptr = (char*) &data;
    buffer[offset] = ptr[3];
    buffer[offset+1] = ptr[2];
    buffer[offset+2] = ptr[1];
    buffer[offset+3] = ptr[0];
}

void Buffer::writeUInt32BE(buint32 data, bint64 offset) {
    char *ptr = (char*) &data;
    buffer[offset] = ptr[3];
    buffer[offset+1] = ptr[2];
    buffer[offset+2] = ptr[1];
    buffer[offset+3] = ptr[0];
}

void Buffer::writeInt64BE(bint64 data, bint64 offset) {
    char *ptr = (char*) &data;
    buffer[offset] = ptr[7];
    buffer[offset+1] = ptr[6];
    buffer[offset+2] = ptr[5];
    buffer[offset+3] = ptr[4];
    buffer[offset+4] = ptr[3];
    buffer[offset+5] = ptr[2];
    buffer[offset+6] = ptr[1];
    buffer[offset+7] = ptr[0];
}

void Buffer::writeUInt64BE(buint64 data, bint64 offset) {
    char *ptr = (char*) &data;
    buffer[offset] = ptr[7];
    buffer[offset+1] = ptr[6];
    buffer[offset+2] = ptr[5];
    buffer[offset+3] = ptr[4];
    buffer[offset+4] = ptr[3];
    buffer[offset+5] = ptr[2];
    buffer[offset+6] = ptr[1];
    buffer[offset+7] = ptr[0];
}

void Buffer::copyFrom(const char* buf, bint64 offset) {
    bint64 len = strlen(buf);
    if(offset+len > size) throw "out of bounds";
    memcpy(&(buffer[offset]), buf, len);
}

void Buffer::copyFrom(const char* buf, bint64 len, bint64 offset) {
    if(offset+len > size) throw "out of bounds";
    memcpy(&(buffer[offset]), buf, len);
}

void Buffer::copyFrom(const Buffer &from, bint64 offset) {
    copyFrom(from.buffer, from.size, offset);
}

Buffer Buffer::slice(bint64 start) {
    Buffer out;
    out.alloc(size-start);
    out.copyFrom(buffer, 0);
    return out;
}

void Buffer::concat(Buffer &with) {
    // create new memory and copy all
    char *newBuf = new char[size+with.size];
    
    memcpy(newBuf, buffer, size);
    memcpy(&newBuf[size], with.buffer, with.size);
    delete[] buffer;
    buffer = newBuf;
    size += with.size;
}

// read
bint8 Buffer::readInt8BE(bint64 offset) {
    return buffer[offset];
}

buint8 Buffer::readUInt8BE(bint64 offset) {
    return buffer[offset];
}

bint16 Buffer::readInt16BE(bint64 offset) {
    char out[2];
    out[1] = buffer[offset];
    out[0] = buffer[offset+1];

    return *((bint16*)out);
}

buint16 Buffer::readUInt16BE(bint64 offset) {
    char out[2];
    out[1] = buffer[offset];
    out[0] = buffer[offset+1];

    return *((buint16*)out);
}

bint32 Buffer::readInt32BE(bint64 offset) {
    char out[4];
    out[0] = buffer[offset+3];
    out[1] = buffer[offset+2];
    out[2] = buffer[offset+1];
    out[3] = buffer[offset];

    return *((bint32*)out);
}

buint32 Buffer::readUInt32BE(bint64 offset) {
    char out[4];
    out[0] = buffer[offset+3];
    out[1] = buffer[offset+2];
    out[2] = buffer[offset+1];
    out[3] = buffer[offset];

    return *((buint32*)out);
}

bint64 Buffer::readInt64BE(bint64 offset) {
    char out[8];
    out[0] = buffer[offset+7];
    out[1] = buffer[offset+6];
    out[2] = buffer[offset+5];
    out[3] = buffer[offset+4];
    out[4] = buffer[offset+3];
    out[5] = buffer[offset+2];
    out[6] = buffer[offset+1];
    out[7] = buffer[offset];

    return *((bint64*)out);
}

buint64 Buffer::readUInt64BE(bint64 offset) {
    char out[8];
    out[0] = buffer[offset+7];
    out[1] = buffer[offset+6];
    out[2] = buffer[offset+5];
    out[3] = buffer[offset+4];
    out[4] = buffer[offset+3];
    out[5] = buffer[offset+2];
    out[6] = buffer[offset+1];
    out[7] = buffer[offset];

    return *((buint64*)out);
}

void Buffer::shiftRight(bint64 shiftBy) {
    // assuming size is sufficiently large for the shift
    bint64 r = size + shiftBy - 1;
    bint64 l = size - 1;

    while(l >= 0) {
        // swap
        buffer[l] = buffer[r] ^ buffer[l];
        buffer[r] = buffer[l] ^ buffer[r];
        buffer[l] = buffer[l] ^ buffer[r];

        l--;
        r--;
    }
}