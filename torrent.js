'use strict';

const fs = require('fs');
const bencode = require('bencode');
const crypto = require('crypto');
const bignum = require('bignum');
const getPeers = require('./getPeers');


class Torrent {
    constructor(torrentFilePath, destinationPath) {
        this.torrent = fs.readFileSync(torrentFilePath);
        this.BLOCK_LEN = Math.pow(2, 14);
        this.destinationPath = destinationPath;
        this.infoHash = _infoHash();  // TODO check if this is performed after torrent id read from memory
        this.size = _size();
    }

    _infoHash() {
        const info = bencode.encode(this.torrent.info);
        return crypto.createHash('sha1').update(info).digest();
    }

    _size() {
        const size = this.torrent.info.files ?
        this.torrent.info.files.map(file => file.length).reduce((a, b) => a + b) :
        this.torrent.info.length;

        return bignum.toBuffer(size, {size: 8});
    }

    download() {
        
    }

}

module.exports = {
   Torrent: Torrent 
};