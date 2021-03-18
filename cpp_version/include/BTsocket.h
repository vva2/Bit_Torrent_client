#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h> 
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "type.h"
// #include <iostream>


// port range 6881-6889

/*
    struct addrinfo {
        int ai_flags; // AI_PASSIVE, AI_CANONNAME, etc.
        int ai_family; // AF_INET, AF_INET6, AF_UNSPEC
        int ai_socktype; // SOCK_STREAM, SOCK_DGRAM
        int ai_protocol; // use 0 for "any"
        size_t ai_addrlen; // size of ai_addr in bytes
        struct sockaddr *ai_addr; // struct sockaddr_in or _in6
        char *ai_canonname; // full canonical hostname
        
        
        struct addrinfo *ai_next;
        // linked list, next node
    };

    struct sockaddr {
        unsigned short sa_family;  // address family, AF_xxx
        char sa_data[14];  // 14 bytes of protocol address
    };

    struct sockaddr_in {
        short int sin_family;  // Address family, AF_INET
        unsigned short int sin_port;  // Port number
        struct in_addr sin_addr;  // Internet address
        unsigned char sin_zero[8]; // Same size as struct sockaddr
    };

    struct in_addr {
        uint32_t s_addr; // that's a 32-bit int (4 bytes)
    };

    struct sockaddr_in sa; // IPv4
    struct sockaddr_in6 sa6; // IPv6

    inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr)); // IPv4
    inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr)); //IPv6

    // inet_pton() returns -1 on error, or 0 if the address is messed up

    // IPv4 conversion from bytes to dot seperated string:
    char ip4[INET_ADDRSTRLEN];
    struct sockaddr_in sa;
    something
    // space to hold the IPv4 string
    // pretend this is loaded with
    inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
    printf("The IPv4 address is: %s\n", ip4);

    struct hostent {
        char  *h_name;            // official name of host
        char **h_aliases;         // alias list
        int    h_addrtype;        // host address type
        int    h_length;          // length of address
        char **h_addr_list;       // list of addresses
    }

    The gethostbyname() function returns a structure of  type  hostent  for
    the given host name.  Here name is either a hostname or an IPv4 address
    in standard dot notation (as for inet_addr(3)).

    // IMP
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    int getaddrinfo(const char *node, // e.g. "www.example.com" or IP
        const char *service, // e.g. "http" or port number
        const struct addrinfo *hints,
        struct addrinfo **res);
    

    #include <sys/types.h>
    #include <sys/socket.h>
    int socket(int domain, int type, int protocol);


    // bind
    #include <sys/types.h>
    #include <sys/socket.h>
    int bind(int sockfd, struct sockaddr *my_addr, int addrlen);

    recv() can return 0 . This can mean only one thing: the remote side has closed the
    connection on you! A return value of 0 is recv() ’s way of letting you know this has
    occurred.

    // UDP :  sendto
    int sendto(int sockfd, const void *msg, int len, unsigned int flags, const struct sockaddr *to, socklen_t tolen);

    // UDP : recvfrom
    int recvfrom(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr *from, int *fromlen);


*/


// TODO add functionality of recvNBytes in both TCP and UDP
namespace BTSocket {
    class Udp {
        int32 sockfd;
        struct addrinfo *servInfo, hints;  // do some tests on how to clear servInfo

        public:
            Udp();
            ~Udp();
            int32 sendTo(char *data, int32 dataLen, const char *address, const char* port);
            int32 recvFrom(char *buf, int32 bufLen);
            int32 recvNBytes();
            void end();
            bool isOpen();
    };

    class Tcp {
        public:
            int32 sockfd;
            struct addrinfo *servInfo, hints;

            Tcp();
            ~Tcp();
            int32 connectTo(const char *address, const char *port);
            int32 sendData(char *data, int32 dataLen);
            int32 recvData(char *buf, int32 bufLen);
            int32 recvNBytes(char *buf, int32 n);  // waits until Nbytes are recieved and returns buffer
            void end();  // closes socket and sets sockfd to -1
            bool isOpen();  // checks if sockfd != -1;
    };
};

