/**********************************
 * FILE NAME: DNet.h
 *
 * DESCRIPTION: Network classes header file
 **********************************/

#ifndef DNET_H
#define DNET_H

#include "stdincludes.h"
#include "Member.h"

class DNet {
private:
    int sockfd;
    int port;
    
public:
    DNet() {};
    DNet(DNet &anotherDNet);
    DNet& operator = (DNet &anotherDNet);
    virtual ~DNet() {
        close(sockfd);
    };
    
    int DNinit();
    int DNinfo(std::string & addr);
    int DNsend(Address * toaddr, std::string data);
    int DNrecv(Address & fromaddr, std::string & data, int timeout);
    int DNcleanup();
    
    // TODO: For EC, we need to handle encrypt & decrypt
};

#endif /* DNET_H */