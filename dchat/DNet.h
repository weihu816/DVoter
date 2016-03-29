/**********************************
 * FILE NAME: DNet
 *
 * DESCRIPTION: Network classes header file
 **********************************/

#ifndef DNET_H
#define DNET_H

#include "stdincludes.h"
#include "Member.h"

class DNet
{
private:
    int sockfd;
    std::queue<q_elt> queue;
public:
    DNet() {};
    // DNet(DNet &anotherDNet);
    // DNet& operator = (DNet &anotherDNet);
    virtual ~DNet() {};

    std::string DNinit();
    int DNsend(Address *toaddr, std::string data);
    int DNrecv(int (* enq)(void *, char *, int), struct timeval *t, int times, void *queue);
    int DNcleanup();
    
    bool enqueue(void *buffer, int size) {
        q_elt element(buffer, size);
        queue.emplace(element);
        return true;
    }
};

#endif /* DNET_H */
