#include "../include/util.h"

void printHex(char buf[], int n) {
    char hex[] = "0123456789abcdef";
    for(int i=0;i<n;i++) {
        printf("%c%c", hex[(buf[i]>>4)&0x0f], hex[buf[i]&0x0f]);
    }
    printf("\n");
}

int64 getFileLen(FILE* fp) {
    fseek(fp, 0, SEEK_END);
    int64 len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    return len;
}

Buffer genId() {
    Buffer id(20);
    // id.copyFrom("-G3sdbyq-", 0);
    id.copyFrom("-VV1010-", 0);
    return id;
}

int32 getRandom() {
    srand(time(NULL));
    return rand();
}

bool isPieceAvailable(char* bitField, uint32 index) {
    return (bitField[index >> 3] & (1 << (7 - (index & 0x7))));
}

bool getBitField(char *bitField, uint32 index) {
    return (bitField[index >> 3] & (1 << (7 - (index & 0x7))));;
}

void setBitField(char *bitField, uint32 index) {
    bitField[index >> 3] = bitField[index >> 3] | (1 << (7 - (index & 0x7))); 
}

void resetBitField(char *bitField, uint32 index) {
    bitField[index >> 3] = bitField[index >> 3] & (~(1 << (7 - (index & 0x7)))); 
} 