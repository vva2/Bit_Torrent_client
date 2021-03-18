'use strict';

const dgram = require('dgram');
const Buffer = require('buffer').Buffer;
const urlParse = require('url').parse;
const {Peer} = require("./peer.js");
const crypto = require('crypto');
const util = require('./util');

const CONNECT = 0;
const ANNOUNCE = 1;

// TODO find a way to close this socket after download

class Helper {
    constructor() {
        this.socket = dgram.createSocket('udp4');
    }

    getPeers(torrent, callback) {
        const announceUrl = torrent.announceUrl;
        const [connReq, transactionId] = this.buildConnReq();
        console.log('connection request is: ', connReq);
        this.udpSend(connReq, announceUrl);

        // use transactionId for matching the recieved packet
        // TODO message can come in parts : handle it
        this.socket.on('message', response => {
            console.log("recieved response...");
            const responseType = this.respType(response);
            if(responseType === CONNECT) {
                console.log("recived CONNECT response...");
                const connResp = this.parseConnResp(response);
                const announceReq = this.buildAnnounceReq(connResp.connectionId, transactionId, torrent);
                this.udpSend(announceReq, announceUrl);
            }
            else if(responseType === ANNOUNCE){
                console.log("recieved ANNOUNCE response...");
                // TODO check #seeders == #peers
                callback(this.parseAnnounceResp(response).peers);
            }
            // TODO handle else condition here
        });
    }


    buildConnReq() {
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


    udpSend(message, rawUrl, callback=()=>{}) {
        // TODO implement retry
        const url = urlParse(rawUrl);
        console.log("requesting connect...");
        this.socket.send(message, 0, message.length, url.port, url.hostname, callback);
    }


    respType(response) {
        // verify announce or connect response
        const action = response.readUInt32BE(0);
        if(action == 0) return CONNECT;
        else if(action == 1) return ANNOUNCE;
    }


    parseConnResp(response) {
        return {
            action: response.readUInt32BE(0),
            transactionId: response.readUInt32BE(4),
            connectionId: response.slice(8)
        };
    }


    buildAnnounceReq(connectionId, transactionId, torrent, port=6881) {
        const buf = Buffer.allocUnsafe(98);

        // connection id
        connectionId.copy(buf, 0);
        // action
        buf.writeUInt32BE(1, 8);
        // transaction id
        transactionId.copy(buf, 12);
        // info hash
        torrent.infoHash.copy(buf, 16);
        // peerId
        util.genId(20).copy(buf, 36);
        // downloaded
        Buffer.alloc(8).copy(buf, 56);
        // left
        torrent.size.copy(buf, 64);
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

    parseAnnounceResp(response) {
        function group(buf, groupSize) {
            let res = [];
            console.log(buf.toString('utf8'));
            for(let i = 0;i < buf.length;i += groupSize) {
                res.push(new Peer({
                    ip: buf.slice(i, i+4).join('.'),
                    port: buf.readUInt16BE(i+4)
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

    end() {
        this.socket.end();
    }
}

    


module.exports = {
    Helper: Helper
};