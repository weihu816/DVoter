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
    Member memberNode;

public:
    // Need to know the centralized server address
    DNode ();
    Member * getMemberNode() {
        return &memberNode;
    }
    int recvLoop();
    
    void nodeStart(char *servaddrstr, short serverport);
    
    int initThisNode(Address *joinaddr);
    int introduceSelfToGroup(Address *joinAddress);
    int finishUpThisNode();
    
    void checkMessages();
    
    void nodeLoopOps();
    
    void nodeLoop();
    
    Address getJoinAddress();
};

#endif /* DNODE_H */

