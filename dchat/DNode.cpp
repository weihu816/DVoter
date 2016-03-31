/**********************************
 * FILE NAME: DNode.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of DNode class functions.
 **********************************/

#include "DNode.h"

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int DNode::recvLoop() {
//    if ( memberNode.bFailed ) {
//        return false;
//    }
//    else {
//        return dnet->DNrecv(enqueueWrapper, NULL, 1, &(memberNode.mp1q));
//    }
    return false;
}


/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
int DNode::nodeStart() {
    
    // Self booting routines
    if( initThisNode() == -1 ) {
#ifdef DEBUGLOG
        std::cerr << "init_thisnode failed. Exit." << std::endl;
#endif
        return FAILURE;
    }
    
    if( !introduceSelfToGroup(joinAddress) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        std::cerr << "Unable to join self to group. Exiting." << std::endl;
#endif
        return FAILURE;
    }
    
    return SUCCESS;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int DNode::initThisNode() {
    std::string ip = memberNode->address->ip;
    int port = memberNode->address->port;
    
    memberNode->bFailed = false;
    memberNode->inited = true;
    memberNode->inGroup = false;
    // node is up!
    memberNode->nnb = 0;
    memberNode->heartbeat = 0;
    memberNode->pingCounter = TFAIL;
    memberNode->timeOutCounter = -1;
    memberNode->memberList.clear();
    return 0;
}


/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int DNode::introduceSelfToGroup(Address *joinaddr) {
    // TODO
    std::string msg;
    
    std::string self_addr = memberNode->address->getAddress();
    std::string join_addr = joinaddr->getAddress();

    if (self_addr.compare(join_addr) == 0) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        std::cout << "Starting up group..." + self_addr << std::endl;
#endif
        memberNode->inGroup = true;
    } else {
        // Join
#ifdef DEBUGLOG
        std::cout << "Trying to join..." + self_addr << std::endl;
#endif
        // create JOINREQ message: format of data is {struct Address myaddr}
        
        // send JOINREQ message to introducer member
        dNet->DNsend(joinaddr, msg);
    }
    return 1;
}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int DNode::finishUpThisNode() {
    
    return 0;
}


/**
 * FUNCTION NAME: sendMsg
 *
 * DESCRIPTION: Send a chat message
 */
void DNode::sendMsg(std::string msg) {
    std::string leader_address = memberNode->getLeaderAddress();
    std::string self_address = memberNode->address->getAddress();
    if (leader_address.compare(self_address)) { // I'm the leader (sequencer)
        multicastMsg(msg);
    } else { // Send Multicast request to the sequencer
        std::stringstream ss;
        ss << D_CHAT << ":" << username << ":" << msg;
        Address leader_addr(leader_address);
        dNet->DNsend(&leader_addr, ss.str());
#ifdef DEBUGLOG
        std::cout << "Send message: " + ss.str() << " to (Leader): " << leader_addr.getAddress() << std::endl;
#endif
    }
}


/**
 * FUNCTION NAME: multicastMsg
 *
 * DESCRIPTION: Multicast the message to all the members
 */
void DNode::multicastMsg(std::string msg) {
    int seq = this->getNextSeqNum();
    std::stringstream ss;
    auto list = memberNode->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        ss.clear();
        ss << D_MSG << ":" << seq << ":" << msg;
        Address addr((*iter).getAddress());
        dNet->DNsend(&addr, ss.str());
#ifdef DEBUGLOG
        std::cout << "Multicast message: " + ss.str() << " to: " << addr.getAddress() << std::endl;
#endif
    }
}


/**
 * FUNCTION NAME: recvMsg
 *
 * DESCRIPTION:
 */
std::string DNode::recvMsg() {
    Address fromAddr;
    std::string content;
#ifdef DEBUGLOG
    std::cout << "Handling message: " + content << " from: " << fromAddr.getAddress() << std::endl;
#endif
    if (dNet->DNrecv(fromAddr, content) == SUCCESS) {
        char *cstr = new char[content.length() + 1];
        strcpy(cstr, content.c_str());
        char * msg_type = strtok(cstr, ":");
        if (strcmp(msg_type, D_CHAT)) {
            char * name= strtok (NULL, ":");
            char * msg= strtok (NULL, ":");
            
        } else if (strcmp(msg_type, D_MSG)) {
            char * seq_num = strtok (NULL, ":");
            char * msg = strtok (NULL, ":");
        }
        
        delete [] cstr;
    }
    return NULL;
}
