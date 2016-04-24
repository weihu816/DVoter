/**********************************
 * FILE NAME: DNode.h
 *
 * DESCRIPTION: Membership protocol run by this Node
 *              Header file of DNode class.
 **********************************/

#ifndef DNODE_H
#define DNODE_H

#include "stdincludes.h"
#include "Member.h"
#include "DNet.h"
#include "Queue.h"

class Handler;

/**
 * CLASS NAME: DNode
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class DNode {
    
private:
    int id = 0;
    // Member information
    std::string username;
    Member * member_node;
    // Network Layer
    std::string join_address;
    DNet * dNet;
    // Message Container
    blocking_queue<std::string> message_chat_queue;
    
public:
    // largest sequence number proposed by q to group g
    int P = 0;
    // largest agreed sequence number q has observed so far for group g, id#pid
    std::pair<int, int> A = std::make_pair(0, ::getpid());
    
    // multicst_queue will be initilized using a sequence number init_seen from the leader
    blocking_priority_queue * queue;
    
    DNode(std::string name, std::string join_addr="");
    
    // DNode related
    int nodeStart();
    int initThisNode();
    int nodeLeave();
    
    // Communication
    void multicast(std::string type);
    void sendMsg(std::string msg);
    void addMessageToDisplay(std::string msg);
    
    // Thread Routine
    int recvLoop();
    std::string msgLoop();
    void nodeLoopOps();
    
    // Membership
    int introduceSelfToGroup(std::string joinAddress);
    void initMemberList(std::string member_list);
    void addMember(std::string ip_port, std::string name, bool toPrint);
    void deleteMember(std::string ip_port);
    
    
    Member * getMember();
    std::string getUsername();
    
    virtual ~DNode() {
        if (dNet) delete dNet;
        if (member_node) delete member_node;
        if (queue) delete queue;
    }
    
    int getNextId() {
        return id++;
    }
};

#endif /* DNODE_H */