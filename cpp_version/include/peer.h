#pragma once
#include "util.h"
#include "message.h"
#include "BTsocket.h"
#include "status.h"
#include "piece.h"
#include "priorityQueue.h"
#include "message.h"

#include <mutex>
#include <queue>

#define BLOCK_SIZE 16384  // 2^14 bytes
#define BTBUFFER_SIZE 17000  // block + additional info
#define BTTHRESHOLD -52428800  // 50 MB
#define INF 1000000

class Status;  // forward declaration to avoid circular depandancy

class Peer {
    private:
    public:
        int32 peerIndex;
        int64 priority;  // higher is preferredl set to INF initially
        int32 upSpeed;  // speed in kbps
        int32 downSpeed;
        int64 downLen;  // downloaded length in bytes
        int64 upLen;  // uploaded length in bytes
        int32 unFinishedPieces;

        int32 blockAssigned;  // set to -1 if nothing is assigned
        bool busy;
        bool choked;
        bool blocked;  // true when peer chokes after first unchoking
        char *bitField;  // stored as bits not bytes  bitField[i] = bitField[i >> 3] & (1 << (7-(i & 0x7))) ; bitField of the peer
        char* ip;
        char* port;
        Status *status;  // points to the status of Torrent class ; request next block to download from this
        BTSocket::Tcp *socket;  // tcp socket ;; udp socket is only created temporarily
        Buffer* buffer;
        void (*__writeCallback)(PieceMsg*, Status*);
        // define callback pointer here

        Peer();
        ~Peer();
        void readNextMsg(bool handshake=false);
        void __errorHandler(int64 line, const char *func, const char *msg);
        void init(char *ip, char* port, Status *status, int32 peerIndex, void (__writeCallback)(PieceMsg*, Status*));  // copy to new memory dont map
        void __updateHave(uint32 &pieceIndex);
        void __updateBitField(char *bitFieldArr);
        void msgHandler(RecvdMsg* msg);
        void download();
        bool _continue();
        void connect();
        void closeConn();
        void upload(ReqMsg* msg);

};

namespace PSpace {
    void pDownload(Peer *peer);
};