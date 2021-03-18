#include "../include/BTsocket.h"

BTSocket::Udp::Udp() {
    printf("UDP contructor called...\n");
    sockfd = -1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  // TODO use AF_INET if does not work
    hints.ai_socktype = SOCK_DGRAM;
}

BTSocket::Udp::~Udp() {
    if(servInfo) delete servInfo;
    close(sockfd);
}

int32 BTSocket::Udp::sendTo(char *data, int32 dataLen, const char *address, const char* port) {
    // here assuming address is null terminated
    struct addrinfo* ptr;
    int status;
    
    printf("sending data to address : %s\t port : %s\n", address, port);

    servInfo = NULL;
    if((status = getaddrinfo(address, port, &hints, &servInfo)) != 0) {
        printf("error in gettting address info from address : %s and port : %s\n", address, port);
        return -1;
    }

    printf("successfully got addr info...\n");
    for(ptr = servInfo;ptr != NULL; ptr = ptr->ai_next) {
        if((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1) {
            printf("failed to initialize socket. trying next one...");
            continue;
        }
        break;
    }

    if(ptr == NULL) {
        printf("failed to initialize socket for address : %s port : %s\n", address, port);
        return -1;
    }

    int64 len = 0;


    // data may not be sent in one go
    while(dataLen > 0) {
        len = sendto(sockfd, &data[len], dataLen, 0, ptr->ai_addr, ptr->ai_addrlen);
        if(len == -1) {
            printf("error occured while sending data...\n");
            return -1;
        }

        dataLen -= len;
    }
    freeaddrinfo(servInfo);  // TODO hold on dont delete this yet

    printf("data successfully sent!!!\n");
    return 1;
}

int32 BTSocket::Udp::recvFrom(char *buf, int32 bufLen) {
    struct sockaddr_storage from;
    socklen_t fromLen = sizeof(from);

    // not using the recieved information
    // can verify the sender using from : use servInfo
    int32 readLen = recvfrom(sockfd, buf, bufLen-1, 0, (struct sockaddr *) &from, &fromLen);
    if(readLen == -1) {
        printf("error in recieving\n");
    }

    return readLen;
}

void BTSocket::Udp::end() {
    if(servInfo) freeaddrinfo(servInfo);
    close(sockfd);
    sockfd = -1;
}

bool BTSocket::Udp::isOpen() {
    return sockfd != -1;
}

//////////////////////
// TCP
//////////////////////

BTSocket::Tcp::Tcp() {
    // do nothing
    sockfd = -1;
}

BTSocket::Tcp::~Tcp() {
    end();
    if(servInfo) {
        if(servInfo->ai_addr)
            delete servInfo->ai_addr;
        delete servInfo;
    }
    close(sockfd);
}

int32 BTSocket::Tcp::connectTo(const char *address, const char *port) {
    // assuming address and port is null terminated string only
    // bind first
    struct addrinfo* ptr;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  // TODO use AF_INET if does not work
    hints.ai_socktype = SOCK_STREAM;

    servInfo = NULL;
    if((status = getaddrinfo(address, port, &hints, &servInfo)) != 0) {
        printf("error in gettting address info from address : %s and port : %s\n", address, port);
        return -1;
    }

    printf("successfully got addr info...\n");
    for(ptr = servInfo;ptr != NULL; ptr = ptr->ai_next) {
        if((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1) {
            printf("failed to initialize socket. trying next one...");
            continue;
        }
        break;
    }

    if(ptr == NULL) {
        printf("failed to initialize socket for address : %s port : %s\n", address, port);
        return -1;
    }

    // struct addrinfo *tempServInfo  = new struct addrinfo;
    // memcpy(tempServInfo, ptr, sizeof(addrinfo));
    // tempServInfo->ai_addr = new struct sockaddr;
    // memcpy(tempServInfo->ai_addr, ptr->ai_addr, sizeof(sockaddr));
    // tempServInfo->ai_next = NULL;

    // freeaddrinfo(servInfo);
    // servInfo = tempServInfo;

    servInfo = ptr;
    printf("obtained a socketfd of %d\n", sockfd);

    // // now bind
    // status = bind(sockfd, servInfo->ai_addr, servInfo->ai_addrlen);
    // if(status == -1) {
    //     printf("error in binding to the socket with address %s ; port : %s\n", address, port);
    //     return -1;
    // }

    printf("connecting to the socket...\n");
    // now connect
    status = connect(sockfd, servInfo->ai_addr, servInfo->ai_addrlen);
    if(status == -1) {
        printf("error in connecting to the socket with address %s ; port : %s\n", address, port);
        return -1;
    }

    status = listen(sockfd, 20);
    if(status == -1) {
        printf("error in listening to the socket with address %s ; port : %s\n", address, port);
        return -1;
    }

    status = accept(sockfd, servInfo->ai_addr, &servInfo->ai_addrlen);
    if(status == -1) {
        printf("error in accepting to the socket with address %s ; port : %s\n", address, port);
        return -1;
    }

    close(sockfd);  // close the listening socket
    sockfd = status;  // send and recv socketfd
}

int32 BTSocket::Tcp::sendData(char *data, int32 dataLen) {
    // int send(int sockfd, const void *msg, int len, int flags);
    int64 len = 0;

    // data may not be sent in one go
    while(dataLen > 0) {
        len = send(sockfd, &data[len], dataLen, 0);
        if(len == -1) {
            printf("error occured while sending data...\n");
            return -1;
        }

        dataLen -= len;
    }
}

int32 BTSocket::Tcp::recvData(char *buf, int32 bufLen) {
    // int recv(int sockfd, void *buf, int len, int flags);
    // recv() can return 0 . This can mean only one thing: the remote side has closed the connection on you

    // can verify the sender using from : use servInfo
    int32 readLen = recv(sockfd, buf, bufLen, 0);
    if(readLen == -1) {
        printf("error in recieving\n");
        return -1;
    }
    else if(readLen == 0) {
        printf("connection closed");
        end();
        return -1;
    }

    return readLen;
}

int32 BTSocket::Tcp::recvNBytes(char *buf, int32 N) {
    int32 readLen = 0;
    int32 offset = 0;
    while(offset < N) {
        readLen = recv(sockfd, &buf[offset], N-offset, 0);
        if(readLen == 0) {
            printf("connection closed");
            end();
            return -1;
        }
        else if(readLen == -1) {
            printf("error in recieving\n");
            return -1;  // probably close the connection
        }
    
        offset += readLen;
    }

    return offset;
}

void BTSocket::Tcp::end() {
    if(servInfo) freeaddrinfo(servInfo);
    close(sockfd);
    sockfd = -1;
}

bool BTSocket::Tcp::isOpen() {
    return sockfd != -1;
}