'use strict';

const fs = require('fs');
const crypto = require('crypto');
const bignum = require('bignum');
const bencode = require('bencode');
const {Helper} = require('./helper');
const {File} = require('./file');

// realtive path, start, end, 
// TODO can add multiple torrent functionality and priority of torrents
// once download is done close all opened sockets
// TODO take care of dropped connections: call a 'round' for all idle peers
// TODO try to implement rarest first: maintain a numberOfPeers[nPieces] array to get the priority
// TODO take care of floating point division


class Torrent {
    constructor(torrent, destinationPath) {
        this.torrent = torrent;
        this.destinationPath = destinationPath;
        this.torrent.infoHash = this._infoHash();
        this.torrent.size = this._size();
        this.files = [];
        this.peers = [];
        this.nPieces = this.torrent.info.pieces.length/20;
        this.requested = new Array(this.nPieces).fill(false);
        this.downloaded = new Array(this.nPieces).fill(false);
        this.helper = new Helper();
        this.piecesDownloaded = 0;
        this.pieceLength = torrent.info['piece length'];

        this.initFiles();
        console.log(this.torrent);
    }

    _infoHash() {
        const info = bencode.encode(this.torrent.info);
        return crypto.createHash('sha1').update(info).digest();
    }

    _size() {
        let size = 0;

        if(this.torrent.info.files) {
            for(let file of this.torrent.info.files) {
                size += file.length;
            }
        }
        else 
            size = torrent.info.length;

        return bignum.toBuffer(size, {size: 8});
    }

    initFiles() {
        // TODO check how relative paths are organized in torrent file
        let start = 0;
        if(this.torrent.files) {
            // create the root directory
            if(!fs.existsSync(this.torrent.info.name)) {
                fs.mkdirSync(this.torrent.info.name);
            }

            this.torrent.info.files.forEach(file => {
                this.files.push(File(start, start+file.length-1, file.path.toString('utf8')))
                start += file.length;
            });
        }
        else {
            this.files.push(File(start, start+this.torrent.info.length-1, file.info.name));
        }
    }

    start() {
        console.log("get peers...");
        this.helper.getPeers({
            announceUrl: this.torrent.announce.toString('utf8'),
            infoHash: this.torrent.infoHash,
            size: this.torrent.size
        }, peers => {
            console.log("got peers");
            // assuming peers are added in the middle of a download

            peers.forEach(peer => {
                peer.index = this.peers.length;
                this.peers.push(peer);
                this.peers[peer.index].init(this.torrent, this.writeCallback, this.requested, this.downloaded);
            });
        });
    }

    search(x) {
        // binary search the files
        let l = 0;
        let r = files.length-1;
        let m;

        while(l < r) {
            m = l+Math.floor((r-l)/2);
            
            if(this.files[m].start > x)
                r = m-1;
            else if(this.files[m].end < x)
                l = m+1;
            else {
                l = m;
                r = m;
            } 
        }

        return l;
    }   

    writeCallback(block) {
        // write the pieces to the disk and clear the piece from the array
        // handles slicing the pieces according to the files
        // TODO binary search for the file in a loop and write bytes seperately for each file from a piece
        // write the data to filesystem-synchronous way
        // split the data based on file_size's
        // TODO handle addition of extra peers

        let offset = block.pieceIndex*this.torrent.info['piece length']+block.begin;
        let fileIndex = 0;
        let length = 0;
        let file = null;
        while(block.data.length) {
            // select the file to write
            fileIndex = search(offset);
            file = this.files[fileIndex];
            length = (block.data.length > (file.end-file.start+1))? (file.end-file.start+1): block.data.length;

            file.write(offset-file.start, block.data.slice(0, length));
            offset += length;
            block.data = block.data.slice(length);
        }


        // handle dropped connections here
        this.piecesDownloaded++;
        if(this.piecesDownloaded === this.nPieces) {
            this.end();
            return null;
        }
        else if(this.peers[piece.peerIndex].busy === false) {
            this.peers[piece.peerIndex].download();
        }
    }


    end() {
        this.helper.end();
        
        for(let i = 0;i < this.peers.length;i++) {
            this.peers[i].end();
        }
    }
}

module.exports = {
   Torrent: Torrent 
};