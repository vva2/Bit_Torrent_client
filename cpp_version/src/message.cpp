#include "../include/message.h"


Buffer* buildConnReq() {
    Buffer* buf = new Buffer;
    buf->alloc(16);
    buf->writeUInt64BE(0x41727101980, 0);
    buf->writeUInt32BE(0, 8);
    buf->writeUInt32BE(getRandom(), 12);

    return buf;
}

Buffer* buildAnnReq(uint64 connId, uint64 left, char* infoHash, uint16 port) {
    /*
        Offset  Size    Name    Value
        0       64-bit integer  connection_id
        8       32-bit integer  action          1 // announce
        12      32-bit integer  transaction_id
        16      20-byte string  info_hash
        36      20-byte string  peer_id
        56      64-bit integer  downloaded
        64      64-bit integer  left
        72      64-bit integer  uploaded
        80      32-bit integer  event           0 // 0: none; 1: completed; 2: started; 3: stopped
        84      32-bit integer  IP address      0 // default
        88      32-bit integer  key             ? // random
        92      32-bit integer  num_want        -1 // default
        96      16-bit integer  port            ? // should be betwee
        98
    */

    Buffer *buf = new Buffer(98);
    // connection id
    buf->writeUInt64BE(connId, 0);
    // action
    buf->writeUInt32BE(1, 8);
    // transaction id
    buf->writeUInt32BE(getRandom(), 12);
    // info hash
    buf->copyFrom(infoHash, HASH_SIZE, 16);
    // peerId
    buf->copyFrom(genId(), 36);
    // downloaded initialized with 0s

    // left
    buf->writeUInt64BE(left, 64);
    // uploaded set to 0s
    
    // event
    buf->writeUInt32BE(0, 80);
    // ip address
    buf->writeUInt32BE(0, 80);
    // key
    buf->writeUInt32BE(getRandom(), 88);
    // num want
    buf->writeInt32BE(-1, 92);
    // port
    buf->writeUInt16BE(port, 96);

    return buf;
}


// parse
ConnResp* parseConnReq(Buffer *resp) {
    ConnResp* res = new ConnResp;
    res->action = resp->readUInt32BE(0);
    res->transId = resp->readUInt32BE(4);
    res->connId = resp->readUInt64BE(8);

    return res;
}

AnnResp* parseAnnReq(Buffer *resp) {
    /*
        Offset      Size            Name            Value
        0           32-bit integer  action          1 // announce
        4           32-bit integer  transaction_id
        8           32-bit integer  interval
        12          32-bit integer  leechers
        16          32-bit integer  seeders
        20 + 6 * n  32-bit integer  IP address
        24 + 6 * n  16-bit integer  TCP port
        20 + 6 * N
    */
    AnnResp *res = new AnnResp;
    res->action = resp->readUInt32BE(0);
    res->tranId = resp->readUInt32BE(4);
    res->interval = resp->readUInt32BE(8);
    res->leechers = resp->readUInt32BE(12);
    res->seeders = resp->readUInt32BE(16);

    // count nAddr first
    res->nAddr = (resp->size-20)/6;
    res->addr = new Addr[res->nAddr];
    int8 *temp = new int8[50];
    int len;
    for(uint32 i=0;i<res->nAddr;i++) {
        // assuming only ipv4 addresses
        len = sprintf(temp, "%d.%d.%d.%d", (uint8) resp->buffer[20+i*6], (uint8) resp->buffer[20+i*6+1], (uint8) resp->buffer[20+i*6+2], (uint8) resp->buffer[20+i*6+3]);
        res->addr[i].ip = new int8[len+1];
        memcpy(res->addr[i].ip, temp, len);
        res->addr[i].ip[len] = '\0';
        
        // copy port
        len = sprintf(temp, "%d", resp->readUInt16BE(20+i*6+4));
        res->addr[i].port = new int8[len+1];
        memcpy(res->addr[i].port, temp, len);
        res->addr[i].port[len] = '\0';
    }
    delete[] temp;

    return res;
}

Buffer* buildHandshake(char *infoHash) {
    /*
        * handshake: <pstrlen><pstr><reserved><info_hash><peer_id>

        * pstrlen: string length of <pstr>, as a single raw byte
        * pstr: string identifier of the protocol
        * reserved: eight (8) reserved bytes. All current implementations use all zeroes. Each bit in these bytes can be used to change the behavior of the protocol. An email from Bram suggests that trailing bits should be used first, so that leading bits may be used to change the meaning of trailing bits.
        * info_hash: 20-byte SHA1 hash of the info key in the metainfo file. This is the same info_hash that is transmitted in tracker requests.
        * peer_id: 20-byte string used as a unique ID for the client. This is usually the same peer_id that is transmitted in tracker requests (but not always e.g. an anonymity option in Azureus).
    */

    Buffer *buf = new Buffer(68);
    // pstrlen
    buf->writeUInt8BE(19, 0);
    // pstr
    buf->copyFrom("BitTorrent protocol", 1);
    // reserved
    buf->writeUInt32BE(0, 20);
    buf->writeUInt32BE(0, 24);
    // info hash
    buf->copyFrom(infoHash, HASH_SIZE, 28);
    // peer id
    buf->copyFrom(genId(), 48);
    return buf;
}

Buffer* buildKeepAlive() {
    /*
        keep-alive: <len=0000>
    */
    Buffer *buf = new Buffer(4);
    return buf;
}

Buffer* buildChoke() {
    /*
        choke: <len=0001><id=0>
    */
    Buffer *buf = new Buffer(5);
    // length
    buf->writeUInt32BE(1, 0);
    // id
    buf->writeUInt8BE(0, 4);
    return buf;
}

Buffer* buildUnChoke() {
    /*
        unchoke: <len=0001><id=1>
    */
    Buffer *buf = new Buffer(5);
    // length
    buf->writeUInt32BE(1, 0);
    // id
    buf->writeUInt8BE(1, 4);
    return buf;
}

Buffer* buildInterested() {
    /*
        interested: <len=0001><id=2>
    */
    Buffer *buf = new Buffer(5);
    // length
    buf->writeUInt32BE(1, 0);
    // id
    buf->writeUInt8BE(2, 4);
    return buf;
}

Buffer* buildUnInterested() {
    /*
        not interested: <len=0001><id=3>
    */
    Buffer *buf = new Buffer(5);
    // length
    buf->writeUInt32BE(1, 0);
    // id
    buf->writeUInt8BE(3, 4);
    return buf;
}

Buffer* buildHave(uint32 pieceIndex) {
    /*
        have: <len=0005><id=4><piece index>
    */
    Buffer *buf = new Buffer(9);
    // length
    buf->writeUInt32BE(5, 0);
    // id
    buf->writeUInt8BE(4, 4);
    // piece index
    buf->writeUInt32BE(pieceIndex, 5);
    return buf;
}

// TODO modify this
Buffer* buildBitField(char *bitField, uint32 bitFieldLen) {
    /*
        bitfield: <len=0001+X><id=5><bitfield>
    */
    Buffer *buf = new Buffer(5+bitFieldLen);
    // length
    buf->writeUInt32BE(bitFieldLen + 1, 0);
    // id
    buf->writeUInt8BE(5, 4);
    // bitfield
    buf->copyFrom(bitField, bitFieldLen, 5);
    return buf;
}

Buffer* buildRequest(ReqMsg* payload) {
    /*
        request: <len=0013><id=6><index><begin><length>
    */
    Buffer *buf = new Buffer(17);
    // length
    buf->writeUInt32BE(13, 0);
    // id
    buf->writeUInt8BE(6, 4);
    // piece index
    buf->writeUInt32BE(payload->index, 5);
    // begin
    buf->writeUInt32BE(payload->begin, 9);
    // length
    buf->writeUInt32BE(payload->length, 13);
    return buf;
}


void buildPiece(PieceMsg* payload) {
    /*
        piece: <len=0009+X><id=7><index><begin><block>
    */
    // memory reuse
    Buffer* buf = payload->block;
    buf->shiftRight(13);

    buf->writeUInt32BE(payload->block->size + 9, 0);
    // id
    buf->writeUInt8BE(7, 4);
    // piece index
    buf->writeUInt32BE(payload->index, 5);
    // begin
    buf->writeUInt32BE(payload->begin, 9);
    // block  :: already copied to buffer
    buf->size += 13;
}

Buffer* buildCancel(ReqMsg* payload) {
    /*
        cancel: <len=0013><id=8><index><begin><length>
    */
    Buffer *buf = new Buffer(17);
    // length
    buf->writeUInt32BE(13, 0);
    // id
    buf->writeUInt8BE(8, 4);
    // piece index
    buf->writeUInt32BE(payload->index, 5);
    // begin
    buf->writeUInt32BE(payload->begin, 9);
    // length
    buf->writeUInt32BE(payload->length, 13);
    return buf;
}

Buffer* buildPort(uint16 port) {
    /*
        port: <len=0003><id=9><listen-port>
        The port message is sent by newer versions of the Mainline that implements a DHT tracker. The listen port is the port this peer's DHT node is listening on. This peer should be inserted in the local routing table (if DHT tracker is supported).
    */
    Buffer *buf = new Buffer(7);
    // length
    buf->writeUInt32BE(3, 0);
    // id
    buf->writeUInt8BE(9, 4);
    // listen-port
    buf->writeUInt16BE(port, 5);
    return buf;
}

bool isHandshake(Buffer *msg) {
    uint32 pstrlen = msg->readUInt8BE(0);

    return ((pstrlen+49) == msg->size) && (pstrlen == 19) && (strncmp(&(msg->buffer[1]), "BitTorrent protocol", pstrlen));
}

// TODO also slice buffer to appropriate length
RecvdMsg* parseMsg(Buffer* msg, bool handshake) {
    if(msg->size < 4) return NULL;  // unknown type
    
    // assuming no handshake msg here 
    RecvdMsg* res = new RecvdMsg;
    
    if(handshake) {
        res->type = MThandshake;
        res->msg = NULL;
        return res;
    }
    
    uint32 len = msg->readUInt32BE(0);
    uint8 id = (len == 0)? 0: msg->readUInt8BE(4);

    switch(id) {
        case MTkeepAlive:
            res->type = MTkeepAlive;
            res->msg = NULL;
            break;
        case MTchoke:
            res->type = MTchoke;
            res->msg = NULL;
            break;
        case MTunchoke:
            res->type = MTunchoke;
            res->msg = NULL;
            break;
        case MTinterested:
            res->type = MTinterested;
            res->msg = NULL;
            break;
        case MTnotInterested:
            res->type = MTnotInterested;
            res->msg = NULL;
            break;
        case MThave:
            {
                res->type = MThave;
                HaveMsg* ptr = new HaveMsg;
                ptr->pieceIndex = msg->readUInt32BE(5);
                res->msg = ptr;
            }
            break;
        case MTbitField:
            {
                res->type = MTbitField;
                BitFieldMsg *ptr = new BitFieldMsg;
                ptr->bitField = new Buffer(msg->size-5);
                memcpy(ptr->bitField, &(msg->buffer[5]), msg->size-5);
                res->msg = ptr;
            }
            break;
        case MTrequest:
            {
                res->type = MTrequest;
                ReqMsg* ptr = new ReqMsg;
                ptr->index = msg->readUInt32BE(5);
                ptr->begin = msg->readUInt32BE(9);
                ptr->length = msg->readUInt32BE(13);
                res->msg = ptr;
            }
            break;
        case MTpiece:
            {
                res->type = MTpiece;
                PieceMsg *ptr = new PieceMsg;
                ptr->index = msg->readUInt32BE(5);
                ptr->begin = msg->readUInt32BE(9);
                ptr->block = new Buffer(len-9);
                memcpy(ptr->block, &(msg->buffer[13]), len-9);
                res->msg = ptr;
            }
            break;
        case MTcancel:
            {
                res->type = MTcancel;
                CancelMsg* ptr = new CancelMsg;
                ptr->index = msg->readUInt32BE(5);
                ptr->begin = msg->readUInt32BE(9);
                ptr->length = msg->readUInt32BE(13);
                res->msg = ptr;
            }
            break;
        case MTport:
            {
                res->type = MTport;
                PortMsg* ptr = new PortMsg;
                ptr->port = msg->readUInt16BE(5);
                res->msg = ptr;
            }
            break;
        default:
            // error
            res->type = MTerror;
            res->msg = NULL;
    }

    return res;
}

