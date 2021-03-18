#include "../include/torrent.h"

void TSpace::writeCallback(PieceMsg *msg, Status *status) {
    // find files associated with this piece and write to disk
    // update dPieces, dLen, requested, downloaded, bitfield

    // first find the files using a binary search
    int64 l = ((status->pieces[msg->index].start)*BLOCK_SIZE) + msg->begin;  // start in bytes
    int64 r = l+msg->block->size-1;  // end of block in bytes
    int32 fileIndex;
    int32 blockSize;  // in bytes
    uint32 blockOff = 0;

    fileIndex = FSpace::__BSearch(status->files, status->nFiles, l);
    while(l <= r) {
        // calculate offset and blockSize bytes in the file selected
        blockSize = min(r, status->files[fileIndex].end)-l+1;
        status->files[fileIndex].write(&msg->block->buffer[blockOff], blockSize, l-status->files[fileIndex].start);
        blockOff += blockSize;
        l += blockSize;
        fileIndex++;
    }

    // update the parameters
    status->writeUpdate(msg, (status->pieces[msg->index].start+(msg->begin/BLOCK_SIZE)));
}


/* torrent class */
Torrent::Torrent(const char *destPath, int32 nThreads) {
    // initialize a scoket
    // initialize the base folders
    // initlize the Files array
    // does nothing
    int32 n = strlen(destPath);
    
    this->destPath = new char[n+1];
    strcpy(this->destPath, destPath);

    infoHash = NULL;
    announceUrl = NULL;
    announceLen = -1;
    sleepTime = 60000;  // 60 seconds
    this->nThreads = nThreads;
    status = new Status;
    _threads = NULL;
    _queue = NULL;
}


void Torrent::parseInfo(BENode* info) {
    int64 len;
    BEString *s;
    char *fullPath = new char[1000];
    int64 baseLen = 0;  // base path length

    // name
    s = (BEString*) (*info)["name"]->obj;
    len = s->size;
    status->name = new char[len+1];
    memcpy(status->name, s->value, len);
    status->name[len] = '\0';

    // piece length
    status->pieceLen = ((BEInt*) (*info)["piece length"]->obj)->value;

    // files and pieces
    // total length : get from files
    // for each piece start and end block indices, pieceIndex and checksum

    // files
    if((*info)["files"]) {
        memcpy(fullPath, s->value, len);
        baseLen = len; 
        fullPath[baseLen] = '/';  // TODO change this to support windows
        baseLen++;

        debug.info("full path : ");
        debug.info(fullPath, baseLen);
        debug.info("\n");

        // multiple files
        status->totalLen = 0;
        BEListNode *head = ((BEList*) (*info)["files"]->obj)->begin;
        int i = 0;

        status->nFiles = ((BEList*) (*info)["files"]->obj)->size();
        status->files = new File[status->nFiles];

        while(head) {
            len = ((BEInt*) ((*(head->value))["length"])->obj)->value;
            s = (BEString*) (*(*(head->value))["path"])[0]->obj;

            memcpy(&fullPath[baseLen], s->value, s->size);
            fullPath[baseLen+s->size] = '\0';
            status->files[i].init(fullPath, status->totalLen, status->totalLen+len-1);

            head = head->next;            
            i++;
            status->totalLen += len;
        }
    }
    else {
        // one file
        status->totalLen = ((BEInt*) (*info)["length"]->obj)->value;
        status->nFiles = 1;
        status->files = new File[status->nFiles];

        // initialize the files array;
        status->files[0].init(status->name, 0, status->totalLen-1);
    }
    
    // pieces
    status->nPieces = (((BEString*) (*info)["pieces"]->obj)->size)/HASH_SIZE;
    status->pieces = new Piece[status->nPieces];
    s = (BEString*) (*info)["pieces"]->obj;  // TODO set only when available

    status->nBlocks = 0;
    uint32 blocksPerPiece = ((status->pieceLen+BLOCK_SIZE-1)/BLOCK_SIZE);
    for(int i = 0; i < (status->nPieces-1); i++) {
        status->pieces[i].pieceIndex = i;
        status->pieces[i].start = status->nBlocks;
        status->nBlocks += blocksPerPiece;
        status->pieces[i].end = status->nBlocks-1;
        status->pieces[i].blocksLeft = blocksPerPiece;
        status->pieces[i].checksum = new char[HASH_SIZE];
        
        memcpy(status->pieces[i].checksum, &((s->value)[i*HASH_SIZE]), HASH_SIZE);
    }
    // last piece
    status->pieces[status->nPieces-1].pieceIndex = status->nPieces-1;
    status->pieces[status->nPieces-1].start = status->nBlocks;
    len = status->totalLen-(status->nPieces-1)*(status->pieceLen);
    status->nBlocks += (len+BLOCK_SIZE-1)/BLOCK_SIZE;
    status->pieces[status->nPieces-1].end = status->nBlocks-1;
    status->pieces[status->nPieces-1].checksum = new char[HASH_SIZE];
    status->pieces[status->nPieces-1].blocksLeft = status->pieces[status->nPieces-1].end - status->pieces[status->nPieces-1].start + 1;
    memcpy(status->pieces[status->nPieces-1].checksum, &((s->value)[(status->nPieces-1)*HASH_SIZE]), HASH_SIZE);

    
    // initialize downloaded and requested array
    status->_lastBlockLen = (status->nBlocks)*(BLOCK_SIZE)-status->totalLen;
    status->downloaded = new bool[status->nBlocks];
    status->requested = new bool[status->nBlocks];
    status->bitFieldLen = (status->nPieces+7)/8;  // ceil
    status->bitField = new char[status->bitFieldLen];
    memset(status->bitField, 0, status->bitFieldLen);

    // clear memory
    delete[] fullPath;
}


void Torrent::initFromFile(char *filePath) {
    // use bencode parser to generate infoHash
    // decode first
    FILE *fp = fopen(filePath, "r");
    // get length
    int64 fileLen = getFileLen(fp);
    char *str = new char[fileLen];
    int64 len = 0;

    printf("file length: %lld\n", fileLen);
    while((len = (len + fread(&str[len], 1, fileLen, fp))) < fileLen);
    fclose(fp);
    printf("read length : %lld\n", len);

    BENode* decoded = new BENode;
    SHA1 *sha1 = new SHA1();
    char *encodedInfo;

    // decode and encode info dictionary to get the infoHash
    BEDecode(str, decoded);
    len = BEEncode((*decoded)["info"], &encodedInfo);
    
    // match info hash using online hash calculator
    infoHash = new char[HASH_SIZE];
    sha1->Input((unsigned char*) encodedInfo, len);
    sha1->Result(infoHash);

    printf("info hash of the torrent file is: \n");
    printHex(infoHash, HASH_SIZE);

    // free heap
    delete sha1;
    delete[] str;
    delete[] encodedInfo;
    // extract other info
    // announceUrl
    announceLen = ((BEString*) (*decoded)["announce"]->obj)->size;
    announceUrl = new char[announceLen+1];
    memcpy(announceUrl, ((BEString*) (*decoded)["announce"]->obj)->value, announceLen);
    announceUrl[announceLen] = '\0';

    printf("announce url : %s\n", announceUrl);

    parseInfo((*decoded)["info"]);
    getPeersFromFile();

    // adjust threads to speed up
    nThreads = min(nThreads, status->nPeers);
    _threads = new std::thread*[nThreads];
    _queue = new Peer*[nThreads];

    printf("using nThreads : %d\n", nThreads);

    delete decoded;
}

void Torrent::getPeersFromFile() {
    // setup an udp connection and request send a handshake
    // build handshake => recieve hanshake => 
    // initialize the socket for udp
    // parse  announce url
    Buffer *buffer = new Buffer;
    buffer->alloc(BTBUFFER_SIZE);  // > 2000 peers : one time
    buffer->size = 0;  // manually adjust


    LUrlParser::ParseURL parsed = LUrlParser::ParseURL::parseURL(announceUrl);
    BTSocket::Udp *sock = new BTSocket::Udp;     
    Buffer *connReq = buildConnReq();
    int32 len;

    printf("parsed msg : \nip: %s\nport : %s\n", parsed.host_.c_str(), parsed.port_.c_str());

    if(parsed.port_ == "") parsed.port_ = "80";

    printf("connection request : ");
    printHex(connReq->buffer, connReq->size);

    debug.info("sending connect request : \n");
    printf("sending a packet of size : %lld\n", connReq->size);
    printf("host : %s\nport : %s\n", parsed.host_.c_str(), parsed.port_.c_str());
    len = sock->sendTo(connReq->buffer, connReq->size, parsed.host_.c_str(), parsed.port_.c_str());
    if(len == -1) {
        debug.error("error in sending data....\n");
        exit(1);
    }

    len = sock->recvFrom(buffer->buffer, BTBUFFER_SIZE);
    if(len == -1) {
        debug.error("error in recieving data....\n");
        exit(1);
    }
    debug.info("recieved connect response\n");
    buffer->size = len;

    ConnResp* connResp = parseConnReq(buffer);  // TODO check if the action is 0
    Buffer *annReq = buildAnnReq(connResp->connId, status->totalLen, infoHash);

    debug.info("sending announce request...\n");
    len = sock->sendTo(annReq->buffer, annReq->size, parsed.host_.c_str(), parsed.port_.c_str());
    if(len == -1) {
        debug.error("error in sendig announce request....\n");
        exit(1);
    }

    len = sock->recvFrom(buffer->buffer, BUFFER_SIZE);
    if(len == -1) {
        debug.error("error in recieving data....\n");
        exit(1);
    }
    debug.info("recieved announce response\n");
    buffer->size = len;

    AnnResp *annResp = parseAnnReq(buffer);
    printf("peers found : %d\n", annResp->nAddr);

    // initilize peers
    status->nPeers = annResp->nAddr;
    status->peers = new Peer[annResp->nAddr];
    for(int i = 0;i < annResp->nAddr; i++) {
        status->peers[i].init(annResp->addr[i].ip, annResp->addr[i].port, status, i, TSpace::writeCallback);
    }
    debug.info("initialized %d peers...\n", annResp->nAddr);

    delete sock;
    delete connResp;
    delete annReq;
    delete connReq;
    delete annResp;
    delete buffer;
}

void Torrent::getPeersFromMagnet() {
    // setup an udp connection and request send a handshake
    // parse url first
    
}

void Torrent::download() {
    // initialize the priority queue
    // pq of peer pointers and pq of piece pointers
    uint32 peersNeeded, i;
    Peer* selected;  // selected peer
    for(int i=0;i<status->nPieces;i++) {
        status->piecepq.push(&status->pieces[i]);
    }

    for(int i=0;i<status->nPeers;i++) {
        peerpq.push(&status->peers[i]);
    }

    printf("first time initializing the queue...\n");
    for(int i=0;i<nThreads;i++) {
        _queue[i] = peerpq.top();
        peerpq.pop();

        _queue[i]->busy = true;
        
        _threads[i] = new std::thread(PSpace::pDownload, _queue[i]);
        _threads[i]->detach();

        status->incrActivePeers();  // by 1
    }


    printf("using %d threads\n", nThreads);
    // TODO adjust timeout based on number of peers available and other parameters
    // run a while loop until download is unfinished
    while(!status->isDone()) {
        // check if active peers != nThreads
        peersNeeded = nThreads - status->getActivePeers();

        printf("peersNeeded: %u\n", peersNeeded);

        // iterate through the peers array and find inactive peers and replace them with the unused peers
        i = 0;
        while(peersNeeded && i < nThreads) {
            // select inactive peer and exchange it with the unused peer from the pq
            if(!_queue[i]->busy) {
                // peer inactive
                // make a choice what to do with the peer based on the peer data
                // upToDown ratio, speed, choked
                peerpq.push(_queue[i]);
                selected = peerpq.top();
                peerpq.pop();  // dont keep the active peers in the pq

                printf("found idle peer with index : %d\n", selected->peerIndex);

                _queue[i] = selected;
                // download in a the ith thread
                // assuming ith thread is free at this time
                delete _threads[i];
                selected->busy = true;
                
                // PSpace::pDownload(selected);

                _threads[i] = new std::thread(PSpace::pDownload, selected);
                _threads[i]->detach();

                status->incrActivePeers();  // by 1
                peersNeeded--;
            }
            i++;
        }

        // sleep for a timeout before next iterarion
        // will give more room for peer download threads   
        sleep(sleepTime);
    }

    debug.info("downloaded successfully\n");
}   

Torrent::~Torrent() {
    debug.info("torrent destructor called\n");
    if(destPath) delete[] destPath;
    if(infoHash) delete[] infoHash;
    if(announceUrl) delete[] announceUrl;
    if(status) delete status;
    if(_threads) delete[] _threads;
    if(_queue) delete[] _queue;
}