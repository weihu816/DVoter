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

public:
    DNet(DNet &anotherDNet);
    DNet& operator = (DNet &anotherDNet);
    virtual ~DNet();

    void *DNinit(short port);
    int DNsend(Address *toaddr, std::string data);
    int DNsend(Address *toaddr, char *data, int size);
    int DNrecv(int (* enq)(void *, char *, int), struct timeval *t, int times, void *queue);
    int DNcleanup();
};

#endif /* DNET_H */
