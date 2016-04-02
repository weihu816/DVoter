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
#include "DNode.h"
#include "DNet.h"
#include "BlockingQueue.h"

/**
 * Macros
 */
#define TREMOVE 20
#define TFAIL 5

/**
 * Message Types
 */
enum MsgTypes {
    JOINREQ,
    JOINREP,
    DUMMYLASTMSGTYPE
};

/**
 * STRUCT NAME: MessageHdr
 *
 * DESCRIPTION: Header and content of a message
 */
typedef struct MessageHdr {
    enum MsgTypes msgType;
}MessageHdr;


/**
 * CLASS NAME: DNode
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class DNode {
private:
    Member * memberNode;
    DNet * dNet;
    Address * joinAddress = nullptr;
    std::string username;
    int seq_num = 0;
    int seq_num_seen = 0;
    int getNextSeqNum() {
        return seq_num++;
    }
    blocking_queue<std::pair<Address, std::string>> m_queue;
    blocking_queue<std::pair<int, std::string>> message_chat_queue;
    blocking_queue<std::string> message_chat_queue_ready;

public:
    DNode(std::string name) : username(name) {
        dNet = new DNet();
        std::string myAddr = dNet->DNinit();
        memberNode = new Member(myAddr);        // Create Member node
        joinAddress = new Address(myAddr);      // Join address
        std::cout << username << " started a new chat, listening on "
            << memberNode->getAddress() << std::endl;
    }
    
    DNode(std::string name, std::string addr) : username(name) {
        dNet = new DNet();
        std::string myAddr = dNet->DNinit();
        memberNode = new Member(myAddr);        // Create Member node
        joinAddress = new Address(addr);        // JoimyAddressn address
        std::cout << username << " join an existing chat, listening on "
            << memberNode->getAddress() << std::endl;
    }

    Member * getMemberNode() {
        return memberNode;
    }

    int nodeStart();
    int initThisNode();
    int introduceSelfToGroup(Address *joinAddress, bool isSureLeaderAddr);
    int finishUpThisNode(); // Wind up this node and clean up state
    void initMemberList(std::string member_list, std::string leaderAddr);
    void addMember(std::string ip_port, bool isLeader);
    
    int recvLoop();
    void nodeLoop();
   
    static int enqueueWrapper(void *env, char *buff, int size);
    void nodeLoopOps();
    void multicastHeartbeat();
    void sendHeartbeat();
    
    
    void checkMessages();
    void recvHandler(std::pair<Address, std::string>);
    
    void sendMsg(std::string msg);
    void multicastMsg(std::string msg);
    
    void sendHeartbeat();
    void multicastHeartbeat();
    
    virtual ~DNode() {
        delete dNet;
        delete memberNode;
        delete joinAddress;
    }
};

#endif /* DNODE_H */

