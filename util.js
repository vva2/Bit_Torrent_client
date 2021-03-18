'use strict';

const crypto = require('crypto');
const fs = require('fs');
const bencode = require('bencode');

const PEER_ID = "-VV0001-";

function genId(size=20) {
    const id = crypto.randomBytes(size);
    Buffer.from(PEER_ID).copy(id, 0);
    return id;
}

function open(filePath) {
    const torrent = bencode.decode(fs.readFileSync(filePath));
    return torrent;
}

module.exports = {
    genId : genId,
    open : open
};