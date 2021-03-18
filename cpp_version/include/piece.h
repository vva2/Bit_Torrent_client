#pragma once
#include "type.h"
#include <mutex>

// TODO add checksum verification
struct Piece {
    private:
        enum __PieceType {
            __PieceGet,
            __PieceSet
        };

    public:
        int32 pieceIndex;
        int32 start;  // start block index
        int32 end;  // end block index
        int32 blocksLeft;  // to be downloaded
        char *checksum;  // ignore for now
        int64 priority;
        std::mutex mtx;

    private:
        void __helper(__PieceType type, int64 &data) {
            mtx.lock();
            switch(type) {
                case __PieceGet:
                    data = priority;
                    break;
                case __PieceSet:
                    priority = data;
                    break;
            }
            mtx.unlock();
        }
    
    public:
        Piece() {
            priority = 0;  // gives the number of peers has this piece
            checksum = NULL;
        }

        ~Piece() {
            if(checksum) delete[] checksum;
        }
        
        int64 getPriority() {
            int64 res;
            __helper(__PieceGet, res);
            return res;
        }

        void setPriority(int64 newPriority) {
            __helper(__PieceSet, newPriority);
        }
};