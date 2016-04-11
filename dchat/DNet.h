/**********************************
 * FILE NAME: DNet.h
 *
 * DESCRIPTION: Network classes header file
 **********************************/

#ifndef DNET_H
#define DNET_H

#include "stdincludes.h"
#include "Member.h"
#include "Handler.h"

class DNet {
private:
    int sockfd;
    int port;
    Handler * handler;
    
public:
    DNet(Handler * handler);
    DNet(DNet &anotherDNet) = delete;
    DNet& operator = (DNet &anotherDNet) = delete;
    virtual ~DNet() {
        delete handler;
        close(sockfd);
    };
    
    int DNinit();
    int DNinfo(std::string & addr);
    int DNsend(Address * toaddr, std::string data, std::string & ack, int times);
    int DNrecv(Address & fromaddr, std::string & data);
    int DNcleanup();
    void * get_in_addr(struct sockaddr *sa);
    
    // TODO: For EC, we need to handle encrypt & decrypt
    void encrypt(std::string & data);
    void decrypt(std::string & data);
};

#endif /* DNET_H */