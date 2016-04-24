/**********************************
 * FILE NAME: DNet.h
 *
 * DESCRIPTION: Network classes header file
 **********************************/

#ifndef DNET_H
#define DNET_H

#include "stdincludes.h"
#include "Member.h"
#include "Queue.h"
#include "Handler.h"
//#include <cstring>


class DNet {
private:
    int sockfd;
    int port;
    Handler * handler;
    // the leader has a map of queues to process the request accordingly
    // key = ip:port, blocking_priority_queue include the
    std::unordered_map<std::string, blocking_queue<socket_queue_item>> leader_round_table;
    int tableCounter = -1; // the index of member in the memberList  whose message queue to be checked, or -1 means the leader.
    
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
    
    int processByRoundTable();
    void pushToQueue(std::string self_address, std::string msgToSend) {
        //std::cout << "trying push to queue..." << std::endl;
        leader_round_table[self_address].push_back(socket_queue_item(0, nullptr, Address(), msgToSend));
        processByRoundTable(); // pop and process one message
    }

};

#endif /* DNET_H */