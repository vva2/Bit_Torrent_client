#include "../include/status.h"

/* status class */
Status::Status() {
    dPieces = 0;
    dnLen = 0;
    upLen = 0;
    activePeers = 0;
    nPieces = 0;
    pieceLen = 0;
    nBlocks = 0;
    nFiles = 0;
    totalLen = 0;
    nPeers = 0;

    infoHash = NULL;
    downloaded = NULL;
    requested = NULL;
    bitField = NULL;
    files = NULL;
    name = NULL;
    pieces = NULL;
}

Status::~Status() {
    if(infoHash) delete[] infoHash;
    if(downloaded) delete[] downloaded;
    if(requested) delete[] requested;
    if(bitField) delete[] bitField;
    if(files) delete[] files;
    if(name) delete[] name;
    if(pieces) delete[] pieces;
}

bool Status::__verifyChecksum(uint32 &pieceIndex) {
    // read the files belonging to the piece
    // read chunk by chunk and calculate sha1 hash sequencially
    // verify it with the pieces[pieceIndex].checksum
    int64 l = pieces[pieceIndex].start*BLOCK_SIZE;  // start of bytes
    int64 r = (pieceIndex == nPieces-1)? totalLen: (pieces[pieceIndex].end+1)*BLOCK_SIZE;
    char *buf = new char[BTBUFFER_SIZE];  // read 16KB blocks of data to compute the hash
    char *digest = new char[HASH_SIZE];
    SHA1 *sha1 = new SHA1();
    uint32 blockLen;
    uint32 fileIndex = FSpace::__BSearch(files, nFiles, l);
    
    while(l <= r) {
        // read chunks of data from the file and compute hash
        while(l <= files[fileIndex].end) {
            blockLen = min((int64) BTBUFFER_SIZE, min(r, files[fileIndex].end)-l+1);
            files[fileIndex].read(buf, blockLen, l);
            sha1->Input((unsigned char*) buf, blockLen);
            l += blockLen;
        }
        fileIndex++;
    }

    sha1->Result(digest);
    bool res = strncmp(digest, pieces[pieceIndex].checksum, HASH_SIZE);

    delete[] buf;
    delete[] digest;
    delete sha1;

    return res; 
}

void Status::__syncHelper(__StatusFunc type, void* x, void* y, void* out) {
    mtx.lock();
    switch(type) {
        case __SFGetActivePeers:
            *(int32*)out = activePeers;
            break;
        case __SFIncrActivePeers:
            activePeers++;
            break;
        case __SFDecrActivePeers:
            activePeers--;
            break;
        case __SFIsDone:
            *(bool*)out = (dPieces == nPieces);
            break;
        case __SFWriteUpdate:
            // set downloaded to true, requested to false and set piece->blocksLeft
            // update upLen, dPieces
            // when complete piece is downloaded verify checksum
            {
                PieceMsg *msg = *(PieceMsg**)x;  // whole msg may not be needed :: just pass pieceIndex
                int64 blockIndex = *(int64*)y;
                
                _status[blockIndex] = 'd';
                pieces[msg->index].blocksLeft--;
                
                // do checksum verification
                // if(pieces[msg->index].blocksLeft == 0 && __verifyChecksum(msg->index)) {
                //     // TODO this may fail for last block
                //     dnLen += (pieces[msg->index].end - pieces[msg->index].start)*BLOCK_SIZE;
                //     dPieces++;
                // }
                // else {
                //     __clearPiece(msg->index);
                // }
            }
            break;
        case __SFUpdateHave:
            // update piece priority, and unFinishedPieces
            // if piece is already downloaded do nothing
            {
                uint32 pieceIndex = *(uint32*)x;
                int32 *unFinishedPieces = (int32*)y;
                
                if(!getBitField(bitField, pieceIndex)) {
                    (*unFinishedPieces)++;

                    pieces[pieceIndex].priority++;
                    piecepq.push(&pieces[pieceIndex]);    
                }
            }
            break;
        case __SFUpdateBitField:
            // update piece priority, and unFinishedPieces
            // if piece is already downloaded do nothing
            {
                char* peerBitField = *(char**) x;
                int32* unFinishedPieces = (int32*) y;
                int32 c;

                for(int i = 0; i < bitFieldLen; i++) {
                    c = peerBitField[i];

                    // upper
                    if((c & 0x80) && !(bitField[i] & 0x80)) {
                        *(unFinishedPieces)++;
                        pieces[i*8].priority++;
                        piecepq.push(&pieces[i*8]); 
                    }
                    if((c & 0x40) && !(bitField[i] & 0x40)) {
                        *(unFinishedPieces)++;
                        pieces[i*8+1].priority++;
                        piecepq.push(&pieces[i*8+1]); 
                    }
                    if((c & 0x20) && !(bitField[i] & 0x20)) {
                        *(unFinishedPieces)++;
                        pieces[i*8+2].priority++;
                        piecepq.push(&pieces[i*8+2]); 
                    }
                    if((c & 0x10) && !(bitField[i] & 0x10)) {
                        *(unFinishedPieces)++;
                        pieces[i*8+3].priority++;
                        piecepq.push(&pieces[i*8+3]); 
                    }

                    // lower
                    if((c & 0x08) && !(bitField[i] & 0x08)) {
                        *(unFinishedPieces)++;
                        pieces[i*8+4].priority++;
                        piecepq.push(&pieces[i*8+4]); 
                    }
                    if((c & 0x04) && !(bitField[i] & 0x04)) {
                        *(unFinishedPieces)++;
                        pieces[i*8+5].priority++;
                        piecepq.push(&pieces[i*8+5]); 
                    }
                    if((c & 0x02) && !(bitField[i] & 0x02)) {
                        *(unFinishedPieces)++;
                        pieces[i*8+6].priority++;
                        piecepq.push(&pieces[i*8+6]); 
                    }
                    if((c & 0x01) && !(bitField[i] & 0x01)) {
                        *(unFinishedPieces)++;
                        pieces[i*8+7].priority++;
                        piecepq.push(&pieces[i*8+7]); 
                    }
                }
            }
            break;
        case __SFGetBlock:
            // iterate thru node of nodes of the priority queue and select the undownloaded piece that is available with the peer
            // if the piece is downloaded and peer's bitfield is high then decrease unFinishedPieces and reset the peers bitField
            // within the selected piece get the first undownloaded and unrequested block and assign it to the peerIndex requested it
            // update active peers
            // assign block  : set blockAssigned variable to block index
            {
                Peer* peer = &peers[*(uint32*)x];
                Piece* piece;
                bool peerHasPiece;
                int32 n = piecepq.heap.size();
                ReqMsg* res = NULL;

                for(int i=0;(i<n && peer->unFinishedPieces > 0);i++) {
                    piece = piecepq.heap[i];
                    peerHasPiece = getBitField(peer->bitField, piecepq.heap[i]->pieceIndex);

                    if(piece->blocksLeft == 0) {
                        if(peerHasPiece) {
                            peer->unFinishedPieces--;
                            resetBitField(peer->bitField, piece->pieceIndex);
                        }
                    }
                    else if(peerHasPiece) {
                        // select the first undownloaded and unrequested block
                        for(int j=piece->start;j<=piece->end;j++) {
                            if(_status[j] == 'n') {
                                // select this block
                                _status[j] = 'r';
                                activePeers++;
                                peer->blockAssigned = j;  // assign block
                                res = new ReqMsg;
                                res->begin = (j-piece->start)*BLOCK_SIZE;  // offset for the block in the piece
                                res->index = piece->pieceIndex;
                                res->length = (j == nBlocks-1)? _lastBlockLen: BLOCK_SIZE;  // block length
                            }
                        }
                        if(res) break;
                    }
                }

                // map res to the out pointer
                *(ReqMsg**)out = res;  // out here is ReqMsg**
            }
            break; 
        case __SFCancelBlock:
            // reset the block request to null
            // update active peers
            activePeers--;
            _status[*(uint32*)x] = 'n';
            break;
        default:
            printf("unknown case...\n");
            throw "error";   
    }
    mtx.unlock();
}

int32 Status::getActivePeers() {
    int32 res;
    __syncHelper(__SFGetActivePeers, NULL, NULL, &res);
}

void Status::incrActivePeers() {
    __syncHelper(__SFIncrActivePeers);
}

void Status::decrActivePeers() {
    __syncHelper(__SFDecrActivePeers);
}

bool Status::isDone() {
    bool res;
    __syncHelper(__SFIsDone, NULL, NULL, &res);
}

void Status::writeUpdate(PieceMsg *msg, int64 blockIndex) {
    __syncHelper(__SFWriteUpdate, &msg, &blockIndex);
}

void Status::updateHave(uint32 pieceIndex, int32 &unFinishedPieces) {
    __syncHelper(__SFUpdateHave, &pieceIndex, &unFinishedPieces);
}

void Status::updateBitField(char *peerBitField, int32 &unFinishedPieces) {
    __syncHelper(__SFUpdateBitField, &peerBitField, &unFinishedPieces);
}

void Status::__clearPiece(int32 pieceIndex) {
    // reset all blocks : set to not downloaded
    // increase blocksLeft to number of blocks
    pieces[pieceIndex].blocksLeft = pieces[pieceIndex].end-pieces[pieceIndex].start+1;
    for(int i=pieces[pieceIndex].start;i<pieces[pieceIndex].end;i++) {
        _status[i] = 'n';
    }
}

ReqMsg* Status::getBlock(uint32 peerIndex) {
    ReqMsg* out;
    __syncHelper(__SFGetBlock, &peerIndex, NULL, &out);
}

void Status::cancelBlock(int64 blockIndex) {
    __syncHelper(__SFCancelBlock, &blockIndex);
}


