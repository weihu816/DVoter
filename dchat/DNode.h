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
    
    Member * member_node;
    DNet * dNet;
    std::string join_address;
    std::string username;
    int election_status = E_NONE; //not in election
    blocking_queue<std::string> message_chat_queue;

public:
    // multicst_queue will be initilized using a sequence number init_seen from the leader
    holdback_queue * m_queue;

    DNode(std::string name, std::string join_addr="");
    int nodeStart(); // introduce and start functions
    int initThisNode(); // parameter initialization
    int introduceSelfToGroup(std::string joinAddress, bool isSureLeaderAddr);
    int nodeLeave(); // Wind up this node and clean up state
    void initMemberList(std::string member_list, std::string leaderAddr);
    void addMember(std::string ip_port, std::string name);
    void deleteMember(std::string ip_port); // delete a member
    void clearMembers(); //delete all members
    
    int recvLoop();
    std::string msgLoop();
    void nodeLoop();
    
    static int enqueueWrapper(void *env, char *buff, int size);
    void nodeLoopOps(); // this is the operation for heartbeat
    int checkHeartbeat(std::string address);
    
    void startElection();

    void addMessage(std::string msg);
    void recvHandler(std::pair<Address, std::string>);
    
    void sendMsg(std::string msg);
    void multicastMsg(std::string msg, std::string type);
    void sendNotice(std::string type, std::string addr);
    void multicastNotice(std::string type);
    
    Member* getMember();
    DNet* getDNet();
    std::string getUsername();
    int getElectionStatus();
    void updateElectionStatus(int new_status);

    virtual ~DNode() {
        if(dNet) delete dNet;
        if(member_node) delete member_node;
        if (m_queue) delete m_queue;
    }
};

#endif /* DNODE_H */