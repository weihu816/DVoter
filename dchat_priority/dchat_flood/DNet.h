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
#include "Queue.h"

class DNet {
private:
    int sockfd;
    int port;
    Handler * handler;
    blocking_priority_queue msg_obj_queue;
    
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
    
    int findPriorty(std::string recv_msg);
    int processByPriority();

};

#endif /* DNET_H */