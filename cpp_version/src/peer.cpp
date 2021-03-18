#include "../include/peer.h"

Peer::Peer() {
    // do nothing
}

Peer::~Peer() {
    printf("peer destructor called...\n");
    if(ip) delete[] ip;
    if(port) delete[] port;
    if(socket) delete socket;
    if(bitField) delete[] bitField;
    if(status) delete status;
    if(buffer) delete buffer;
}

void Peer::readNextMsg(bool handshake) {
    // if this functions ends without any errors then msg is read properly
    // read 4 bytes first and rest later
    // handle and throw errors
    uint32 len;
    if(handshake) {
        buffer->size = socket->recvNBytes(buffer->buffer, 4);
        len = buffer->readUInt32BE(0);
        buffer->size += socket->recvNBytes(&buffer->buffer[len+49], len);
    }
    else {
        buffer->size = socket->recvNBytes(buffer->buffer, 4);
        len = buffer->readUInt32BE(0);
        if(len) buffer->size += socket->recvNBytes(&buffer->buffer[len+4], len);
    }
}

void Peer::__errorHandler(int64 line, const char *func, const char *msg) {
    closeConn();
    printf("%lld::%s::%s\n", line, func, msg);
    throw "error";
}

void Peer::init(char *ip, char* port, Status *status, int32 peerIndex, void (__writeCallback)(PieceMsg*, Status*)) {
    this->peerIndex = peerIndex;
    priority = INT_MAX;
    upSpeed = 0;
    downSpeed = 0;
    downLen = 0;
    upLen = 0;
    unFinishedPieces = 0;
    blockAssigned = -1;  // initially no block is assigned
    busy = false;
    choked = true;
    blocked = false;
    bitField = new char[status->bitFieldLen];
    memset(bitField, 0, status->bitFieldLen);
    
    this->ip = new char[strlen(ip)+1];
    strcpy(this->ip, ip);

    this->port = new char[strlen(port)+1];
    strcpy(this->port, port);

    this->status = status;
    socket = new BTSocket::Tcp;
    buffer = new Buffer(BTBUFFER_SIZE);
    this->__writeCallback = __writeCallback;
}

void Peer::closeConn() {
    busy = false;  // let main thread know that one thread is free
    if(blockAssigned != -1) status->cancelBlock(blockAssigned);  
    status->decrActivePeers();  // by 1
    socket->end();
}

bool Peer::_continue() {
    // runs on a detached thread so this funciton is more important factor for deciding the end of the thread
    // tell if download can be continued from this peer based on parameters available
    priority = (downLen+downSpeed*10)-(upLen+upSpeed*10);

    if(blocked || priority < BTTHRESHOLD || unFinishedPieces == 0) {
        closeConn();
        return false;
    }
    return true;
}

void Peer::connect() {
    // establishes connection with peer
    // handshake first then unchoke and wait for the peer to unchoke back or till timeout ends
    // once unchoked send interested
    int32 status = socket->connectTo(ip, port);

    if(status == -1)
        __errorHandler(__LINE__, "Peer::connect", "error in connecting at peer...");
}


void Peer::download() {
    printf("at peer download function...\n");
    // torrent sets the timeout before calling
    // check if the connection is establised or not. if not establish it
    if(socket->isOpen()) {
        printf("socket is open...\n");
        socket->connectTo(ip, port);
        // handshake

        printf("building handshake...\n");
        Buffer* buf = buildHandshake(status->infoHash);
        RecvdMsg *rMsg;
        int32 status;

        printf("sending handshake...\n");
        status = socket->sendData(buf->buffer, buf->size);
        if(status == -1) __errorHandler(__LINE__, "Peer::download", "error in sending handshake...");
        delete buf;

        // recieve handshake
        readNextMsg(true);  // read handshake
        rMsg = parseMsg(buffer, true);
        printf("recieved handshake...\n");
        msgHandler(rMsg);  // will send bitField and unchoke msgs
        delete rMsg;

        printf("waiting for unchoke...\n");
        // recieve unchoke
        readNextMsg();
        rMsg = parseMsg(buffer);
        printf("recieved unchoke msg from peer...\n");
        msgHandler(rMsg);  // will send interested msg
        delete rMsg;
    }

    // requests status pointer for next block and downloads it
    // does this continuously until peer chokes the connection or the downToUpRatio is gets smaller than threshold
    // or no response for a period of timeout
    ReqMsg *reqMsg;
    Buffer *buf;
    RecvdMsg *recvdMsg;
    
    while(_continue()) {
        // do not make another request until the last requested block is downloaded
        if(blockAssigned == -1) {
            // request for next block from status
            reqMsg = status->getBlock(peerIndex);
            buf = buildRequest(reqMsg);
            socket->sendData(buf->buffer, buf->size);
            delete reqMsg;
            delete buf;
        }

        // recieve data
        readNextMsg();
        recvdMsg = parseMsg(buffer);
        msgHandler(recvdMsg);

        delete recvdMsg;
        // recieve msg and call msgHandler
    }

    // status will set the requested array for the last block : as last block may not be downloaded
    // pq update is done by the torrent class
}

void Peer::__updateHave(uint32 &pieceIndex) {
    setBitField(bitField, pieceIndex);
    status->updateHave(pieceIndex, unFinishedPieces);  // changes priority of a piece
}

void Peer::__updateBitField(char *bitFieldArr) {
    memcpy(bitField, bitFieldArr, status->bitFieldLen);
    status->updateBitField(bitField, unFinishedPieces);  // changes priority of a pieces
}


void Peer::msgHandler(RecvdMsg* msg) {
    Buffer *buf;
    switch(msg->type) {
        case MThandshake:
            // send bitField
            // can have synchronization issue
            buf = buildBitField(status->bitField, status->bitFieldLen);
            socket->sendData(buf->buffer, buf->size);
            delete buf;

            // send unchoke
            buf = buildUnChoke();
            socket->sendData(buf->buffer, buf->size);
            delete buf;
            break;
        case MTkeepAlive:
            // do nothing
            // TODO implement closing connection after 2 minutes of keep alives
            break;
        case MTchoke:
            buf = buildChoke();
            socket->sendData(buf->buffer, buf->size);
            delete buf;
            choked = true;
            blocked = true;
            break;
        case MTunchoke:
            buf = buildInterested();
            socket->sendData(buf->buffer, buf->size);
            delete buf;
            choked = false;
            blocked = false;
            break;
        case MTinterested:
            // do nothing  :: likely automatically conitinues the request
            break;
        case MTnotInterested:
            buf = buildChoke();
            socket->sendData(buf->buffer, buf->size);
            delete buf;
            blocked = true;
            choked = true;
            break;
        case MThave:
            __updateHave(((HaveMsg*) msg->msg)->pieceIndex);
            break;
        case MTbitField:
            __updateBitField(((BitFieldMsg*) msg->msg)->bitField->buffer);
            break;
        case MTrequest:
            upload((ReqMsg*) msg->msg);
            break;
        case MTpiece:
            __writeCallback((PieceMsg*) msg->msg, status);
            blockAssigned = -1;
            break;
        case MTcancel:
            // TODO handle this properly
            buf = buildChoke();  // placeholder
            // buf = buildCancel();
            socket->sendData(buf->buffer, buf->size);
            delete buf;
            break;
        case MTport:
            // dunno what to do
            break;
        default:
            __errorHandler(__LINE__, "Peer::msgHandler", "unknown msg recieved");
    }
}


void Peer::upload(ReqMsg* msg) {
    // safety check : if the piece actually available with us
    if(!isPieceAvailable(status->bitField, msg->index)) return;

    // find the files containing the block and upload to the peer
    // first find the files using a binary search
    int64 l = ((status->pieces[msg->index].start)*BLOCK_SIZE) + msg->begin;  // start in bytes
    int64 r = l+msg->length-1;  // end of block in bytes
    int32 fileIndex;
    int32 blockSize;  // in bytes
    int32 dataOff = 0;

    fileIndex = FSpace::__BSearch(status->files, status->nFiles, l);
    while(l <= r) {
        // calculate offset and blockSize bytes in the file selected
        blockSize = min(r, status->files[fileIndex].end)-l+1;
        status->files[fileIndex].read(&buffer->buffer[dataOff], blockSize, l-status->files[fileIndex].start);
        
        dataOff += blockSize;
        l += blockSize;
        fileIndex++;
    }

    // update buffer size
    buffer->size = dataOff;

    PieceMsg *pMsg = new PieceMsg;
    pMsg->index = msg->index;
    pMsg->begin = msg->begin;
    pMsg->block = buffer;

    buildPiece(pMsg);
    pMsg->block = NULL;

    // send block to peer
    socket->sendData(buffer->buffer, buffer->size);

    // do not delete buffer as it points to the class buffer variable
    delete pMsg;

    // TODO take note of upload speed too
    // update the parameters
    upLen += dataOff;
}

void PSpace::pDownload(Peer *peer) {
    peer->download();
}