/**********************************
 * FILE NAME: DNode.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of DNode class functions.
 **********************************/

#include "DNode.h"

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void DNode::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr = getJoinAddress();
    
    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
        std::cerr << "init_thisnode failed. Exit." << std::endl;
#endif
        exit(1);
    }
    
    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        std::cerr << "Unable to join self to group. Exiting." << std::endl;
#endif
        exit(1);
    }
    
    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int DNode::initThisNode(Address *joinaddr) {
    std::string ip = memberNode.addr.addr;
    int port = memberNode.addr.port;
    // TODO:
    
    memberNode.bFailed = false;
    memberNode.inited = true;
    memberNode.inGroup = false;
    // node is up!
    memberNode.nnb = 0;
    memberNode.heartbeat = 0;
    memberNode.pingCounter = TFAIL;
    memberNode.timeOutCounter = -1;
    memberNode.memberList.clear();
    return 0;
}


/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int DNode::introduceSelfToGroup(Address *joinaddr) {
    MessageHdr *msg;
    
    std::string self_addr = memberNode.addr.addr;
    std::string join_addr = joinaddr->addr;

    if (self_addr.compare(join_addr) == 0) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        std::cout << "Starting up group..." + self_addr << std::endl;
#endif
        memberNode.inGroup = true;
    } else {
        // Join
#ifdef DEBUGLOG
        std::cout << "Trying to join..." + self_addr << std::endl;
#endif
        // create JOINREQ message: format of data is {struct Address myaddr}
        
        // send JOINREQ message to introducer member
        
    }
    return 1;
}


/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address DNode::getJoinAddress() {
    Address joinaddr;
    
    // TODO
    
    return joinaddr;
}
