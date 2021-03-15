'use strict';

const fs = require('fs');
const crypto = require('crypto');
const bignum = require('bignum');
const bencode = require('bencode');
const {Helper} = require('./helper');
const {File} = require('./file');
const Path = require('path');


// realtive path, start, end, 
// TODO can add multiple torrent functionality and priority of torrents
// once download is done close all opened sockets
// TODO take care of dropped connections: call a 'round' for all idle peers
// TODO try to implement rarest first: maintain a numberOfPeers[nPieces] array to get the priority
// TODO take care of floating point division


class Torrent {
    constructor(torrent, destinationPath="") {
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

        console.log(this.torrent);
        this.initFiles();
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
            size = this.torrent.info.length;

        return bignum.toBuffer(size, {size: 8});
    }

    initFiles() {
        console.log("initializing files...");
        // TODO check how relative paths are organized in torrent file
        let start = 0;

        this.torrent.info.name = this.torrent.info.name.toString('utf8');
        if(this.torrent.info.files) {
            // create the root directory
            if(!fs.existsSync(this.torrent.info.name)) {
                fs.mkdirSync(this.torrent.info.name);
            }

            this.torrent.info.files.forEach(file => {
                const filePath = Path.join(this.destinationPath, this.torrent.info.name, file.path[0].toString('utf8'));
                console.log('filename : ', filePath);
                this.files.push(new File(start, start+file.length-1, filePath));
                start += file.length;
            });
        }
        else {
            const filePath = Path.join(this.destinationPath, this.torrent.info.name);
            this.files.push(new File(start, start+this.torrent.info.length-1, filePath));
        }

        console.log("inintialized files");
        console.log(this.files);
    }

    start() {
        console.log("get peers...");
        this.helper.getPeers({
            announceUrl: this.torrent.announce.toString('utf8'),
            infoHash: this.torrent.infoHash,
            size: this.torrent.size
        }, peers => {
            console.log("got peers size: ", peers);
            // assuming peers are added in the middle of a download

            peers.forEach(peer => {
                peer.index = this.peers.length;
                this.peers.push(peer);
                this.peers[peer.index].init(this.torrent, this.writeCallback, this.requested, this.downloaded, this);
            });
        });
    }

    searchFile(x, this_) {
        // binary search the files
        let l = 0;
        let r = this_.files.length-1;
        let m;

        while(l < r) {
            m = l+Math.floor((r-l)/2);
            
            if(this_.files[m].start > x)
                r = m-1;
            else if(this_.files[m].end < x)
                l = m+1;
            else {
                l = m;
                r = m;
            } 
        }

        return l;
    }   

    writeCallback(block, this_) {
        // write the pieces to the disk and clear the piece from the array
        // handles slicing the pieces according to the files
        // TODO binary search for the file in a loop and write bytes seperately for each file from a piece
        // write the data to filesystem-synchronous way
        // split the data based on file_size's
        // TODO handle addition of extra peers
        // this_ is the "this" operator of actual torrent class
        // https://stackoverflow.com/questions/3541348/javascript-how-do-you-call-a-function-inside-a-class-from-within-that-class

        let offset = block.pieceIndex*this_.pieceLength+block.begin;
        let fileIndex = 0;
        let length = 0;
        let file = null;

        console.log("block: ", block);
        console.log("offset: ", offset);
        while(block.data.length) {
            // select the file to write
            fileIndex = this_.searchFile(offset, this_);
            file = this_.files[fileIndex];
            length = (block.data.length > (file.end-file.start+1))? (file.end-file.start+1): block.data.length;

            file.write(offset-file.start, block.data.slice(0, length));
            offset += length;
            block.data = block.data.slice(length);
        }


        // handle dropped connections here
        this_.piecesDownloaded++;
        if(this_.piecesDownloaded === this_.nPieces) {
            this_.end();
            return null;
        }
        else if(this_.peers[block.peerIndex].busy === false) {
            this_.peers[block.peerIndex].download();
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