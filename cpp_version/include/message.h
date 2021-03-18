#pragma once
#include "util.h"
#include "buffer.h"

// 0 - choke
// 1 - unchoke
// 2 - interested
// 3 - not interested
// 4 - have
// 5 - bitfield
// 6 - request
// 7 - piece
// 8 - cancel
// 9 - port

// TODO memory can be reduced here 
// TODO handle error responses also  : http://www.bittorrent.org/beps/bep_0015.html
// https://wiki.theory.org/index.php/BitTorrentSpecification#Handshake

enum MessageType {
    MThandshake = 20,
    MTkeepAlive = 21,
    MTchoke = 0,
    MTunchoke = 1,
    MTinterested = 2,
    MTnotInterested = 3,
    MThave = 4,
    MTbitField = 5,
    MTrequest = 6,
    MTpiece = 7,
    MTcancel = 8,
    MTport = 9,
    MTerror = 100
};

struct ConnResp {
    uint32 action;
    uint32 transId;
    uint64 connId;
};

struct Addr {
    int8 *ip;  // null terminated string
    int8 *port;  // null terminated string

    Addr() {
        ip = NULL;
        port = NULL;
    }

    ~Addr() {
        if(ip) delete[] ip;
        if(port) delete[] port; 
    }
};

struct AnnResp {
    uint32 action;
    uint32 tranId;
    uint32 interval;  // wait time before next request
    uint32 leechers;
    uint32 seeders;
    uint32 nAddr;
    Addr* addr;  // array of {ip, port}

    AnnResp() {
        addr = NULL;
    }

    ~AnnResp() {
        if(addr) delete[] addr;
    }
};

struct ReqMsg {
    uint32 index;
    uint32 begin;
    uint32 length;
};

struct PieceMsg {
    uint32 index;
    uint32 begin;
    Buffer* block;

    ~PieceMsg() {
        if(block) delete block;
    }
};

struct HaveMsg {
    uint32 pieceIndex;
};

struct BitFieldMsg {
    Buffer* bitField;

    ~BitFieldMsg() {
        if(bitField) delete bitField;
    }
};

struct CancelMsg {
    uint32 index;
    uint32 begin;
    uint32 length;
};

struct PortMsg {
    uint16 port;
};

struct RecvdMsg {
    MessageType type;
    void* msg;

    ~RecvdMsg() {
        if(msg) {
            switch(type) {
                case MThandshake:
                case MTkeepAlive:
                case MTchoke:
                case MTunchoke:
                case MTinterested:
                case MTnotInterested:
                    break;
                case MThave:
                    delete (HaveMsg*) msg;
                    break;
                case MTbitField:
                    delete (BitFieldMsg*) msg;
                    break;
                case MTrequest:
                    delete (ReqMsg*) msg;
                    break;
                case MTpiece:
                    delete (PieceMsg*) msg;
                    break;
                case MTcancel:
                    delete (CancelMsg*) msg;
                    break;
                case MTport:
                    delete (PortMsg*) msg;
                    break;
            }
        }
    }
};


// build
Buffer* buildConnReq();
Buffer* buildAnnReq(uint64 connId, uint64 torrSize, char* infoHash, uint16 port=6881);
Buffer* buildHandshake(char *infoHash);
Buffer* buildKeepAlive();
Buffer* buildChoke();
Buffer* buildUnChoke();
Buffer* buildInterested();
Buffer* buildUnInterested();
Buffer* buildHave(uint32 pieceIndex);  // called by status class
Buffer* buildBitField(char *bitField, uint32 bitFieldLen);
Buffer* buildRequest(ReqMsg* payload);
void buildPiece(PieceMsg* payload);  // void : to save memory here
Buffer* buildCancel(ReqMsg* payload);
Buffer* buildPort(uint16 port);


// parse
ConnResp* parseConnReq(Buffer *resp);
AnnResp* parseAnnReq(Buffer *resp);
bool isHandshake(Buffer *msg);
RecvdMsg* parseMsg(Buffer* msg, bool handshake=false);  // parses all the 10 types of messages
