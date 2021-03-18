#pragma once
#include <string.h>

#define bint8 char
#define buint8 unsigned char
#define bint16 short
#define buint16 unsigned short
#define bint32 int
#define buint32 unsigned int
#define bint64 long long
#define buint64 unsigned long long

// TODO define all input variables to be const

class Buffer {
    private:
    public:
        char *buffer;
        bint64 size;  // read only externally
        
        // constructor
        Buffer();

        // member functions
        void alloc(bint64 len);  // fill with zeros
        void popFront(bint64 len);
        
        // copy from other data
        void copyFrom(const char* buf, bint64 offset);
        void copyFrom(const char* buf, bint64 len, bint64 offset);
        void copyFrom(const Buffer &from, bint64 offset);

        Buffer(const char* data);
        Buffer(bint64 size);  // initialize with capacity

        // copy
        Buffer(const Buffer &from);

        // write
        void writeInt8BE(bint8 data, bint64 offset);
        void writeUInt8BE(buint8 data, bint64 offset);
        void writeInt16BE(bint16 data, bint64 offset);
        void writeUInt16BE(buint16 data, bint64 offset);
        void writeInt32BE(bint32 data, bint64 offset);
        void writeUInt32BE(buint32 data, bint64 offset);
        void writeInt64BE(bint64 data, bint64 offset);
        void writeUInt64BE(buint64 data, bint64 offset);

        Buffer slice(bint64 start);
        void fill(char x);
        void concat(Buffer &with);

        // read
        bint8 readInt8BE(bint64 offset);
        buint8 readUInt8BE(bint64 offset);
        bint16 readInt16BE(bint64 offset);
        buint16 readUInt16BE(bint64 offset);
        bint32 readInt32BE(bint64 offset);
        buint32 readUInt32BE(bint64 offset);
        bint64 readInt64BE(bint64 offset);
        buint64 readUInt64BE(bint64 offset);

        // shift
        void shiftRight(bint64 shiftBy);

        // destructor
        ~Buffer();
};