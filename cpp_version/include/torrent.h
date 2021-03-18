#pragma once
#include "file.h"
#include "peer.h"
#include "bencode.h"
#include "sha1.h"
#include "message.h"
#include "LUrlParser.h"
#include "BTsocket.h"
#include "status.h"
#include "priorityQueue.h"
#include "log.h"
#include "piece.h"

#include <thread>
#include <queue>

#define BUFFER_SIZE 17000  

// send have msg to peers once a piece is downloaded
// make sure buffer size is properly adjusted before read and write
// set uint32 and int32 appropriately
// checksum verification may be slow :: check buffer size , running time tradeof
// add debug
// make sure you are not returning before unlocking mutex
// blockIndex can fit in int32
// dont need a timeout in peer class
// for each peer reset bitField if the piece is downloaded to disk
// adjust nThreads based on number of peers available
// build port is unused
// pieces queue is a min heap
// delete memory properly
// initialially reserve memory for priority queue
// TODO send have messages in between to all active peers
// TODO build a progress bar
// TODO divide variables and functions to private and public
// TODO start peers with inifinite pritority
// maintain an array ok size "K" => current busy peers
// run parallelly k threads for downloading 
// send and recieve requires different threads so k*2 threads in total
// lock socket write and read resource while sending and recieving data
// so peer class has 2 mutex's
// PQ has entities {prority: float, peerId: int}
// a torrent can have 200 peers
// DHT based torrents use only infoHash to download metadata
// differentiate between seeders and leechers : tit-for-tat


// struct  hostent {
//     char    *h_name;        /* official name of host */
//     char    **h_aliases;    /* alias list */
//     int     h_addrtype;     /* host address type */
//     int     h_length;       /* length of address */
//     char    **h_addr_list;  /* list of addresses from name server */
// };

namespace TSpace {
    class myComparator {
        public:
            int operator()(Peer* &a, Peer* &b) {
                return a->priority < b->priority;
            }
    };

    void writeCallback(PieceMsg *msg, Status *status);
};

class Torrent {
    private:
        char* destPath;
    public:
        // torrent info
        char *infoHash;  // SHA1 or SHA256 hash of the torrent info
        char *announceUrl;  // may not be required for magnet links
        int announceLen;
        int32 sleepTime;
        int32 nThreads;

        // additional info
        Status *status;
        // peers
        

        // socket
        priority_queue<Peer*, vector<Peer*>, TSpace::myComparator> peerpq;  // max heap for peers
        std::thread **_threads;
        Peer** _queue;

        // debug
        LOG::Log debug;

        Torrent(const char *destPath, int32 nThreads=10);
        void init();  // TODO : accept only infoHash and get info from DHT
        void getPeersFromFile();
        void getPeersFromMagnet();
        void initFromFile(char *filePath);  // temporary
        void initFromMagnet(char *magnetLink);  // temporary
        void parseInfo(BENode* info);
        void download();

        // copy construct
        Torrent(const Torrent& from) = delete;
        
        // destructor: end sockets delete all heap memory
        ~Torrent();
};

