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
#include "BlockingQueue.h"

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
    Address * join_address = nullptr;
    std::string username;
    int election_status = E_NONE; //not in election

    int seq_num = 0;
    int seq_num_seen = 0;
    int getNextSeqNum() {
        return seq_num++;
    }
    
//    blocking_queue<std::pair<Address, std::string>> m_queue;
    blocking_queue<std::string> message_chat_queue_ready;
    
public:
    // multicst_queue will be initilized using a sequence number init_seen from the leader
    holdback_queue * multicast_queue;
    

    DNode(std::string name, std::string join_addr="");
    int nodeStart(); // introduce and start functions
    int initThisNode(); // parameter initialization
    int introduceSelfToGroup(Address *joinAddress, bool isSureLeaderAddr);
    int finishUpThisNode(); // Wind up this node and clean up state
    void initMemberList(std::string member_list, std::string leaderAddr);
    void addMember(std::string ip_port, std::string name, bool isLeader);
    void deleteMember(MemberListEntry member); // delete a member
    
    int recvLoop();
    void nodeLoop();
    
    static int enqueueWrapper(void *env, char *buff, int size);
    void nodeLoopOps(); // this is the operation for heartbeat
    int checkHeartbeat(std::string address);
    
    void startElection();
    void handleElection(Address fromAddr, std::string notice);

    void checkMessages();
    void recvHandler(std::pair<Address, std::string>);
    
    void sendMsg(std::string msg);
    void multicastMsg(std::string msg, std::string type);
    void sendNotice(std::string type);
    void multicastNotice(std::string type);
    
    Member* getMember();
    DNet* getDNet();
    int getSeqNum();
    std::string getUsername();
    
    virtual ~DNode() {
        delete dNet;
        delete member_node;
        delete join_address;
        delete multicast_queue;
    }
};

#endif /* DNODE_H */