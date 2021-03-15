#pragma once
#include "type.h"
#include "piece.h"
#include "priorityQueue.h"
#include "file.h"
#include "sha1.h"
#include "peer.h"
#include "util.h"

#include <mutex>


// what is the use of downloaded array
// this class is only instanciated once per torrent
// seperate mutex for getActivePeers
// TODO maintain a requested block array for all active peers and update the requested array if the peer connection is closed
// TODO send have messages to all the active peers once a piece is downloaded
//  class for synchronizing the variable access

class Peer;  // forward declaration to avoid circular depandancy

class Status {
    private:
        enum __StatusFunc {
            __SFGetActivePeers,
            __SFIncrActivePeers,
            __SFDecrActivePeers,
            __SFIsDone,
            __SFWriteUpdate,
            __SFUpdateHave,
            __SFUpdateBitField,
            __SFGetBlock,
            __SFCancelBlock
        };
    
    public:
        int32 dPieces;  // downloaded pieces
        int64 dnLen;  // download length
        int64 upLen;  // upload length
        int32 activePeers;
        int32 nPieces;
        int64 pieceLen;
        uint32 nBlocks;  // total number of pieces in terms of blocks
        uint32 nFiles;
        int64 totalLen;
        int32 nPeers;
        int32 bitFieldLen;
        uint32 _lastBlockLen;

        char *infoHash;
        char *_status;  // block status 'd' for downloaded and 'r' for requested 'n' null state
        bool *downloaded;  // # : number of blocks insteead of pieces
        bool *requested;  // # : number of blocks insteead of pieces
        char *bitField;  // bitfield array
        File* files;  // array of all the files in the torrent
        char *name;  // root directory or file name  :: may not be required
        Peer* peers;  // peers array
        Piece *pieces;
        PriorityQueue<Piece*, BPQLONG_MIN, BPQSpace::__greaterThan> piecepq;  // max heap for pieces   TODO make this min heap
        std::mutex mtx;

        Status();
        ~Status();
        bool __verifyChecksum(uint32 &pieceIndex);
        void __syncHelper(__StatusFunc type, void* x=NULL, void* y=NULL, void* out=NULL);  // helps in synchronizing the calls
        int32 getActivePeers();
        void setActivePeers();
        void incrActivePeers();  // increment active peers by one
        void decrActivePeers();  // decrement active peers by one
        bool isDone();
        void writeUpdate(PieceMsg *msg, int64 blockIndex);  // book keeping after downloading a piece and update piece->blocks left
        void updateHave(uint32 pieceIndex, int32 &unFinishedPieces);  // changes priority
        void updateBitField(char *peerBitField, int32 &unFinishedPieces);
        void __clearPiece(int32 pieceIndex);  // resets the piece if checksum does not match
        ReqMsg* getBlock(uint32 peerIndex);
        void cancelBlock(int64 blockIndex);  // resets the requested[blockIndex]
};