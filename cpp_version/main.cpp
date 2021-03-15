#include "include/torrent.h"


// http://www.bittorrent.org/beps/bep_0003.html
// can use pre compiled headers
// limit the number of used threads per torrent [3-4]
// add magnet link parsing functionality
// priority using the downloaded : uploaded ratio
// dont forget pragma once
// rarest first download : use priority queue
// implement retries as specified in : http://www.bittorrent.org/beps/bep_0015.html
// url parse to get announce url and port
// add switches to ask for magnet link
// make sure all the structures clear up the heap memory after use: write Destructors
// remove repeated includes from all the files

int main(int argc, char *argv[]) {
    printf("%d\n", argc);
    char torrentPath[] = "./test-torrents/tears-of-steel.torrent";

    // LOG(torrentPath, strlen(torrentPath));
    if(argc > 1) printf("%s\n", argv[1]);

    Torrent *torrent = new Torrent(".");  // can use stack memory too
    torrent->initFromFile((argc > 1)? argv[1]: torrentPath);
    torrent->download();

    // free heap memory
    delete torrent;

    // read the torrent and parse the info content
    // compute infoHash using sha1
    // udp handshake => get peers
    // initialize the folder structure
    // c++ fs lib
    // initialize a new peer in heap memory
    // call download for each peer in a seperate thread
    // guard the fwrite operation with mutex
    // delete the peer objects after download
}
