'use strict';

const KEEP_ALIVE = 0;
const HANDSHAKE = 1;
const CHOKE = 2;
const UNCHOKE = 3;
const INTERESTED = 4;
const NOT_INTERESTED = 5;
const HAVE = 6;
const BIT_FIELD = 7; 
const REQUEST = 8;
const PIECE = 9;
const CANCEL = 10;
const PORT = 11;


class Message {
    constructor(msg) {
        this.id = null;
        this.parse(msg);
    }

    msgClass(id) {
        return (id == 0)? CHOKE: (id == 1)? UNCHOKE: (id == 2)? INTERESTED: (id == 3)? NOT_INTERESTED: (id == 4)? HAVE: (id == 5)? BIT_FIELD: (id == 6)? REQUEST: (id == 7)? PIECE: (id == 8)? CANCEL: PORT; 
    }

    isHandshake(msg) {
        return msg.length === msg.readUInt8(0) + 49 && msg.toString('utf8', 1) === 'BitTorrent protocol';
    }

    parse(msg) {
        if(this.isHandshake(msg)) {
            this.id = HANDSHAKE;
            return;
        }

        this.size = msg.readInt32BE(0);
        this.id = msg.length == 4? KEEP_ALIVE: this.msgClass(msg.readInt8(4));
        this.payload = msg.length > 5 ? msg.slice(5) : null;

        if(this.id == HAVE) {
            this.payload = {
                pieceIndex: this.payload.readInt32BE(0)
            };
        }
        else if (this.id === REQUEST || this.id === PIECE || this.id === CANCEL) {
            const rest = this.payload.slice(8);

            this.payload = {
                index: this.payload.readInt32BE(0),
                begin: this.payload.readInt32BE(4)
            };

            if (this.id == PIECE)
                this.payload['block'] = rest;
            else
                this.payload['length'] = rest.readInt32BE(0);
        }
    }
};


module.exports = {
    Message: Message,
    KEEP_ALIVE: KEEP_ALIVE,
    HANDSHAKE: HANDSHAKE,
    CHOKE: CHOKE,
    UNCHOKE: UNCHOKE,
    INTERESTED: INTERESTED,
    NOT_INTERESTED: NOT_INTERESTED,
    HAVE: HAVE,
    BIT_FIELD: BIT_FIELD,
    REQUEST: REQUEST,
    PIECE: PIECE,
    CANCEL: CANCEL,
    PORT: PORT
};