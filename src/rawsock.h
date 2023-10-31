#ifndef RAWSOK_H_
#define RAWSOK_H_

#include "pch.h"

#include "gtrace.h"
#include "packet.h"

struct RawSock {
    int socket = 0;
    struct sockaddr_in addr_in_;
    
    bool open(char* interface);
    bool close();
    
    bool sendto(TxPacket *pkt);
};

#endif // RAWSOK_H_
