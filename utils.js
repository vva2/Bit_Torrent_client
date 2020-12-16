'use strict';

const crypto = require('crypto');

const PEER_ID = "-VV0001-";

function genId(size=20) {
    id = crypto.randomBytes(size);
    Buffer.from(PEER_ID).copy(id, 0);
    return id;
}

module.exports = {
    genId : genId
};