'use strict';

const Buffer = require('buffer').Buffer;
const net = require('net');
const util = require('./util');
const {Message, KEEP_ALIVE, HANDSHAKE, CHOKE, UNCHOKE, INTERESTED, NOT_INTERESTED, HAVE, BIT_FIELD, REQUEST, PIECE, CANCEL, PORT} = require('./message');


class Peer {
    constructor({ip, port}) {
        this.ip = ip;
        this.port = port;
        this.choked = true;
        this.busy = false;
        this.BLOCK_LEN = Math.pow(2, 14);
        this.index = -1;  // used for calling continue from writeCallback
        this.queue = new Set();
        this.socket = new net.Socket();
        
        // parameters for block
        this.blockBegin = 0;
        this.blockIndex = -1;
        this.pieceIndex = -1;
        this.downloadedPieceLen = 0;

        // TODO update the pieceIndex  and peerIndex before sending it to writeCallback
        this.writeCallback = null;
        this.downloaded = null;
        this.requested = null;
        this.torrent = null;
    }

    init(torrent, writeCallback, requested, downloaded, torrentThis) {
        this.writeCallback = writeCallback;
        this.torrent = torrent;
        this.requested = requested;
        this.downloaded = downloaded;  // assuming this is always a reference
        this.socket = new net.Socket();
        this.torrentThis = torrentThis;
        
        this.socket.on('error', console.log);
        this.socket.connect(this.port, this.ip, () => {
            console.log("connection established");
            this.socket.write(this.buildHandshake());
        });

        this.onWholeMsg();
    }

    download() {
        if(this.choked || this.busy) return null;  // check just in case

        // find next piece to download from the queue
        // clear the piece
        let pieceIndex = -1;
        
        // iterate through the queue to find unrequested and undownloaded piece
        // pop downloaded pieces
        for(let x of this.queue) {
            if(this.downloaded[x])
                queue.delete(x);
            else if(this.requested[x])
                continue;
            else {
                pieceIndex = x;
                break;
            }
        }

        if(pieceIndex === -1) return null;
        this.requested[pieceIndex] = true;
        this.pieceIndex = pieceIndex;
        this.blockIndex = -1;
        this.blockBegin = 0;
        this.downloadedPieceLen = 0;

        this.busy = true;
        
        this.requestPiece();
    }

    blockLength(pieceIndex, blockIndex) {
        const lastIndex = Math.floor((this.torrent.size+this.torrent['piece length']-1)/this.torrent['piece length'])-1; 
        const pieceLen = (lastIndex === pieceIndex)? this.torrent.size-this.torrent['piece length']*lastIndex: this.torrent['piece length'];
        const lastBlockIndex = Math.floor((pieceLen+this.BLOCK_LEN-1)/this.BLOCK_LEN)-1;
        return (lastBlockIndex === blockIndex)? (pieceLen-(lastBlockIndex*this.BLOCK_LEN)): this.BLOCK_LEN; 
    }

    requestPiece() {
        this.blockIndex++;
        this.socket.write(this.buildRequest({
            index: this.pieceIndex,
            begin: this.blockBegin,
            length: this.blockLength(this.pieceIndex, this.blockIndex)
        }));
    }

    msgHandler(msg) {
        console.log("recieved a msg...", msg);

        if(msg.id === HANDSHAKE) this.socket.write(self.buildInterested());
        else if (msg.id === CHOKE) this.chokeHandler();
        else if (msg.id === UNCHOKE) this.unchokeHandler();
        else if (msg.id === HAVE) this.haveHandler(msg.payload.pieceIndex);
        else if (msg.id === BIT_FIELD) this.bitFieldHandler(msg.payload);
        else if (msg.id === PIECE) this.pieceHandler(msg);
        else if(msg.id === KEEP_ALIVE) this.keepAliveHandler();
        // TODO handle others
    }


    chokeHandler() {
        this.choked = true;
        this.socket.end();  // TODO think about this
    }


    unchokeHandler() {
        // call download here
        this.choked = false;
        console.log("unchoked...");
        this.download();
    }


    haveHandler(pieceIndex) {
        // adds only unique indices
        this.queue.add(pieceIndex);
    }


    bitFieldHandler(payload) {
        for(let i = 0;i < payload.length;i++) {
            for(let j = 0;j < 8;j++) {
                if((payload[i]>>j)&1)
                    this.queue.add(i*8+7-j);
            }
        }
    }


    isDone() {
        const lastIndex = (this.torrent.size+this.torrent['piece length']-1)/this.torrent['piece length']-1; 
        const pieceLen = (lastIndex === this.pieceIndex)? this.torrent.size-this.torrent['piece length']*lastIndex: this.torrent['piece length'];
        const lastBlockIndex = ((pieceLen+this.BLOCK_LEN-1)/this.BLOCK_LEN)-1;
        return this.blockIndex === lastBlockIndex;
    }


    pieceHandler(msg) {
        // request for next block if not downloaded
        // compute block length to request and call buildRequest() 
        // write the built request to socket
        // build offset and call writeCallback
        // change busy = false on when piece downlaod is complete
        // update blockOffset, reset block

        const block = {
            peerIndex: this.index,
            pieceIndex: msg.payload.index,
            blockIndex: this.blockIndex,
            data: msg.payload.block,
            begin: msg.payload.begin
        };

        this.blockBegin += msg.payload.block.length;
        this.downloadedPieceLen += msg.payload.block.length;
        const done = this.isDone();

        if(done) {
            this.busy = false;
            this.downloaded[this.pieceIndex] = true;
            this.requested[this.pieceIndex] = false;
        }

        this.writeCallback(block, this.torrentThis);
        if(done === false) {
            // request next block
            this.requestPiece();
        }
    }

    keepAliveHandler() {
        // TODO close connection after recieving 2-min of keep-alives
        console.log("keep alive...");
        return null;
    }


    onWholeMsg() {
        console.log('onwholemsg...');

        let handshake = true;
        let savedBuf = Buffer.alloc(0);

        this.socket.on('data', recvBuf => {
            console.log("recieved data...", recvBuf.length);
            // msgLen calculates the length of a whole message
            const msgLen = () => handshake ? savedBuf.readUInt8(0) + 49 : savedBuf.readInt32BE(0) + 4;
            savedBuf = Buffer.concat([savedBuf, recvBuf]);

            // TODO handle keep-alive
            while (savedBuf.length >= 4 && savedBuf.length >= msgLen()) {
                this.msgHandler(new Message(savedBuf.slice(0, msgLen())));
                savedBuf = savedBuf.slice(msgLen());
                handshake = false;
            }
        });
    }

    buildHandshake() {
        console.log('build handshake...')

        const buf = Buffer.alloc(68);
        // pstrlen
        buf.writeUInt8(19, 0);
        // pstr
        buf.write('BitTorrent protocol', 1);
        // reserved
        buf.writeUInt32BE(0, 20);
        buf.writeUInt32BE(0, 24);
        // info hash
        this.torrent.infoHash.copy(buf, 28);
        // peer id

        util.genId().copy(buf, 48);
        
        console.log('built handshake...');
        return buf;
    }

    buildKeepAlive() {
        return Buffer.alloc(4);
    }

    buildChoke() {
        const buf = Buffer.alloc(5);
        // length
        buf.writeUInt32BE(1, 0);
        // id
        buf.writeUInt8(0, 4);
        return buf;
    }

    buildunchoke() {
        const buf = Buffer.alloc(5);
        // length
        buf.writeUInt32BE(1, 0);
        // id
        buf.writeUInt8(1, 4);
        return buf;
    }

    buildInterested() {
        const buf = Buffer.alloc(5);
        // length
        buf.writeUInt32BE(1, 0);
        // id
        buf.writeUInt8(2, 4);
        return buf;
    }

    buildUninterested() {
        const buf = Buffer.alloc(5);
        // length
        buf.writeUInt32BE(1, 0);
        // id
        buf.writeUInt8(3, 4);
        return buf;
    }

    buildHave() {
        const buf = Buffer.alloc(9);
        // length
        buf.writeUInt32BE(5, 0);
        // id
        buf.writeUInt8(4, 4);
        // piece index
        buf.writeUInt32BE(payload, 5);
        return buf;
    }

    buildBitField() {
        const buf = Buffer.alloc(14);
        // length
        buf.writeUInt32BE(payload.length + 1, 0);
        // id
        buf.writeUInt8(5, 4);
        // bitfield
        bitfield.copy(buf, 5);
        return buf;
    }

    buildRequest(payload) {
        const buf = Buffer.alloc(17);
        // length
        buf.writeUInt32BE(13, 0);
        // id
        buf.writeUInt8(6, 4);
        // piece index
        buf.writeUInt32BE(payload.index, 5);
        // begin
        buf.writeUInt32BE(payload.begin, 9);
        // length
        buf.writeUInt32BE(payload.length, 13);
        return buf;
    }

    buildPiece(payload) {
        const buf = Buffer.alloc(payload.block.length + 13);
        // length
        buf.writeUInt32BE(payload.block.length + 9, 0);
        // id
        buf.writeUInt8(7, 4);
        // piece index
        buf.writeUInt32BE(payload.index, 5);
        // begin
        buf.writeUInt32BE(payload.begin, 9);
        // block
        payload.block.copy(buf, 13);
        return buf;
    }

    buildCancel() {
        const buf = Buffer.alloc(17);
        // length
        buf.writeUInt32BE(13, 0);
        // id
        buf.writeUInt8(8, 4);
        // piece index
        buf.writeUInt32BE(payload.index, 5);
        // begin
        buf.writeUInt32BE(payload.begin, 9);
        // length
        buf.writeUInt32BE(payload.length, 13);
        return buf;
    }

    buildPort(payload) {
        const buf = Buffer.alloc(7);
        // length
        buf.writeUInt32BE(3, 0);
        // id
        buf.writeUInt8(9, 4);
        // listen-port
        buf.writeUInt16BE(payload, 5);
        return buf;
    }
};

module.exports = {
    Peer: Peer
};