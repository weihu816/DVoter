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
    int seqNum;

public:
    DNode(std::string name) : username(name) {
        dNet = new DNet();
        std::string myAddr = dNet->DNinit();
        memberNode = new Member(myAddr);        // Create Member node
        joinAddress = new Address(myAddr);      // Join address
    }
    
    DNode(std::string name, std::string addr) : username(name) {
        dNet = new DNet();
        std::string myAddr = dNet->DNinit();
        memberNode = new Member(myAddr);        // Create Member node
        joinAddress = new Address(addr);        // JoimyAddressn address
    }

    Member * getMemberNode() {
        return memberNode;
    }

    int nodeStart();
    int initThisNode();
    int introduceSelfToGroup(Address *joinAddress);
    int finishUpThisNode(); // Wind up this node and clean up state
    
    int recvLoop();
    static int enqueueWrapper(void *env, char *buff, int size);
    void nodeLoopOps();
    void checkMessages();
    bool recvCallBack(void *env, char *data, int size);
    void nodeLoop();
        
    void sendMsg(std::string msg);
    std::string recvMsg();
    
    virtual ~DNode() {
        delete dNet;
        delete memberNode;
        delete joinAddress;
    }
};

#endif /* DNODE_H */

