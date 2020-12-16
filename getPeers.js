'use strict';

const dgram = require('dgram');
const Buffer = require('buffer').Buffer;
const urlParse = require('url').parse;
const Peer = require("./peer.js");
const crypto = require('crypto');
const utils = require('./utils');

const CONNECT = 0;
const ANNOUNCE = 1;


function getPeers(torrent, callback) {
    const socket = dgram.createSocket('udp4');
    const announceUrl = torrent.announce.toString('utf8');
    const [connReq, transactionId] = buildConnReq();
    udpSend(socket, connReq, announceUrl);

    // use transactionId for matching the recieved packet
    socket.on('message', response => {
        const responseType = respType(response);
        if(responseType === CONNECT) {
            const connResp = parseConnResp(response);
            const announceReq = buildAnnounceReq(connResp.connectionId, transactionId, torrent);
            udpSend(socket, announceReq, announceUrl);
        }
        else if(responseType === ANNOUNCE){
            callback(parseAnnounceResp(response).peers);
        }
        // TODO handle else condition here
    });
}


function buildConnReq(transactionId) {
    const buf = Buffer.alloc(16);

    // connection id
    buf.writeUInt32BE(0x417, 0);
    buf.writeUInt32BE(0x27101980, 4);
    // action
    buf.writeUInt32BE(0, 8);
    
    const transactionId = crypto.randomBytes(4);
    // transaction id
    transactionId.copy(buf, 12);

    return [buf, transactionId];
}


function udpSend(socket, message, rawUrl, callback=()=>{}) {
    const url = urlParse(rawUrl);
    socket.send(message, 0, message.length, url.port, url.host, callback);
}


function respType(response) {
    // verify announce or connect response
    const action = response.readUInt32BE(0);
    if(action == 0) return CONNECT;
    else if(action == 1) return ANNOUNCE;
}


function parseConnResp(response) {
    return {
        action: response.readUInt32BE(0),
        transactionId: response.readUInt32BE(4),
        connectionId: response.slice(8)
    };
}


function buildAnnounceReq(connectionId, transactionId, torrent, port=6881) {
    const buf = Buffer.allocUnsafe(98);

    // connection id
    connectionId.copy(buf, 0);
    // action
    buf.writeUInt32BE(1, 8);
    // transaction id
    transactionId.copy(buf, 12);
    // info hash
    torrent.infoHash().copy(buf, 16);
    // peerId
    utils.genId(20).copy(buf, 36);
    // downloaded
    Buffer.alloc(8).copy(buf, 56);
    // left
    torrent.size().copy(buf, 64);
    // uploaded
    Buffer.alloc(8).copy(buf, 72);
    // event
    buf.writeUInt32BE(0, 80);
    // ip address
    buf.writeUInt32BE(0, 80);
    // key
    crypto.randomBytes(4).copy(buf, 88);
    // num want
    buf.writeInt32BE(-1, 92);
    // port
    buf.writeUInt16BE(port, 96);

    return buf;
}   

function parseAnnounceResp(response) {
    function group(buf, groupSize) {
        let res = [];
        for(let i = 0;i < buf.length;i += groupSize) {
            res.push(Peer({
                ip: buf.slice(i, i+4).join('.'),
                port: buf.readUInt32BE(i+4)
            }));
        }
        return res;
    }

    return {
        action: response.readUInt32BE(0),
        transactionId: response.readUInt32BE(4),
        interval: response.readUInt32BE(8),
        leechers: response.readUInt32BE(12),
        seeders: response.readUInt32BE(16),
        peers: group(response.slice(20), 6)
    };
}


module.exports = {
    getPeers: getPeers
};