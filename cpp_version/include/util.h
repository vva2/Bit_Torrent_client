#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "type.h"
#include "buffer.h"

#define HASH_SIZE 20  //  20 bytes for SHA1 hash and 32 bytes for SHA256
// torrent handler
// parse magnet uri and torrent files to TorrentInfo structure
// isOnline using libcurl  ???




// extracts infoHash and possibly announceUrl from the torrent magnet link or file
void printHex(char buf[], int n);
int64 getFileLen(FILE* fp);
Buffer genId();
int32 getRandom();
bool isPieceAvailable(char* bitField, uint32 index);
void setBitField(char *bitField, uint32 index);
bool getBitField(char* bitField, uint32 index);
void resetBitField(char *bitField, uint32 index);