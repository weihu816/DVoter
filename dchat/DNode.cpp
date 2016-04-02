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
    if ( memberNode->bFailed ) {
        return false;
    } else {
        Address addr;
        std::string data;
        dNet->DNrecv(addr, data); // CODE COMMENT: add in current time stamp to allow heartbeat tracking? -> update heartbeatList or leaderHeartbeatTracker
        std::pair<Address, std::string> new_pair(addr, data);
        m_queue.push(new_pair);
    }
    return false;
}


/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void DNode::nodeLoop() {
    if (memberNode->bFailed) {
        return;
    }
    // Check my messages
    checkMessages();
    // Wait until you're in the group...
    if( !memberNode->inGroup ) {
        return;
    }
    // ...then jump in and share your responsibilites!
    nodeLoopOps(); // CODE COMMENT: no message recved would block forever: separate?
    return;
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
    if( initThisNode() == FAILURE ) {
        std::cout << "init_thisnode failed. Exit." << std::endl;
        return FAILURE;
    }
    if( !introduceSelfToGroup(joinAddress, false) ) {
        finishUpThisNode();
        // TODO
        std::cout << "Sorry no chat is active on " << memberNode->getAddress()
            << ", try again later." << std::endl;
        return FAILURE;
    }
    std::cout << "Succeed, current users:" << std::endl; // TODO
//    printMembers();
    // std::cout << username << memberNode->getAddress() << " (Leader)"; // TODO: who is leader
    // std::cout << "Waiting for others to join" << std::endl;
    return SUCCESS;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int DNode::initThisNode() {
    // std::string ip = memberNode->address->ip;
    // int port = memberNode->address->port;
    memberNode->bFailed = false;
    memberNode->inited = true;
    memberNode->inGroup = false;
    // node is up!
    memberNode->nnb = 0;
    memberNode->heartbeat = 0;
    memberNode->pingCounter = TFAIL;
    memberNode->timeOutCounter = -1;
    memberNode->memberList.clear();
    return SUCCESS;
}


/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int DNode::introduceSelfToGroup(Address *joinaddr, bool isSureLeaderAddr) {
    std::string self_addr = memberNode->address->getAddress();
    std::string join_addr = joinaddr->getAddress();

    if (self_addr.compare(join_addr) == 0) {
        // I am the group booter (first process to join the group). Boot up the group
        std::cout << username << " started a new chat, listening on ";
        std::cout << memberNode->getAddress() << std::endl;
        memberNode->inGroup = true;
        addMember(join_addr, true);
    } else {
        std::cout << username << " joining a new chat on " << join_addr << ", listening on ";
        std::cout << memberNode->getAddress() << std::endl;
        // Requst to join by contacting the member
        std::string msg = std::string(D_JOINREQ) + ":" + std::to_string(joinaddr->port);
        if (dNet->DNsend(joinaddr, msg) != SUCCESS) return FAILURE;
        // Receive member lists from the leader, then save the leader address
        Address address;
        std::string message;
        // TODO: what if message lost blocking:timeout
        if (dNet->DNrecv(address, message) != SUCCESS) return FAILURE;
        // send JOINREQ message to introducer member
        size_t index = message.find(":");
        std::string msg_type = message.substr(0, index);
        if (msg_type.compare(D_JOINLEADER) == 0) { // CHAT:Bob:"Hello"
            if (isSureLeaderAddr) return FAILURE; // Leader should return list
            std::string ip_port = message.substr(index+1);
            Address new_addr(ip_port);
            this->introduceSelfToGroup(&new_addr, true); // Connect to leader
        } else if (msg_type.compare(D_JOINLIST) == 0) {
            std::string member_list = message.substr(index+1);
            initMemberList(member_list, address.getAddress());
            memberNode->inGroup = true;
        } else {
            return FAILURE;
        }
    }
    return SUCCESS;
}

/**
 * FUNCTION NAME: initMemberList
 *
 * DESCRIPTION:
 */
void DNode::initMemberList(std::string member_list, std::string leaderAddr) {
    if (member_list.empty()) return;
    char * cstr = new char[member_list.length() + 1];
    std::string addr;
    strcpy(cstr, member_list.c_str());
    std::string ip(strtok(cstr, ":"));
    std::string port(strtok(NULL, ":"));
    addr = ip + ":" + port;
    addMember(addr, addr.compare(leaderAddr)==0);
    char * pch;;
    while ((pch = strtok(NULL, ":")) != NULL) {
        ip = std::string(pch);
        port = std::string(strtok(NULL, ":"));
        addr = ip + ":" + port;
        addMember(addr, addr.compare(leaderAddr)==0);
    }
}

/**
 * FUNCTION NAME: addMember
 *
 * DESCRIPTION:
 */
void DNode::addMember(std::string ip_port, bool isLeader){
#ifdef DEBUGLOG
    std::cout << "DNode::addMember: " << ip_port << std::endl;
#endif
    MemberListEntry entry(ip_port); // memberList with no such boolean?
    memberNode->memberList.push_back(entry);
    if (isLeader) memberNode->leaderAddr = new Address(ip_port);
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
 *              If the current node is the leader, call multicast directly
 *              There is a thread continuing calling this method
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
 *              TODO: only leaders can call this function
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
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void DNode::checkMessages() {
    // Pop waiting messages from DNode's queue
    std::pair<Address, std::string> _pair = m_queue.pop();
    recvHandler(_pair);
    return;
}


/**
 * FUNCTION NAME: recvMsg
 *
 * DESCRIPTION: Message handler for different message types
 *              TODO: what if it receives CHAT:Bob:"Hello", but the current node is not the leader
 *              MSG:1:"Bob: Hello"
 *              CHAT:Bob:"Hello"
 *              TODO: what if the heartbeat gets delayed: update the heartBeat book with any message recved?
 */
void DNode::recvHandler(std::pair<Address, std::string> addr_content) {
    Address fromAddr = addr_content.first;
    std::string content = addr_content.second;
#ifdef DEBUGLOG
    std::cout << "Handling message: " + content << " from: " << fromAddr.getAddress() << std::endl;
#endif
    char * cstr = new char[content.length() + 1];
    strcpy(cstr, content.c_str());
    char * msg_type = strtok(cstr, ":");
    if (strcmp(msg_type, D_CHAT)) {
        
        // Multicast message - CHAT:Bob:"Hello"
        std::string name_recv(strtok (NULL, ":"));
        std::string msg_recv(strtok (NULL, ":"));
        std::string message = name_recv + ":: " + msg_recv;
        multicastMsg(message);

    } else if (strcmp(msg_type, D_MSG)) {
        
        // Display message - MSG:1:"Bob: Hello"
        int seq_recv = atoi(strtok (NULL, ":"));
        std::string msg_recv(strtok (NULL, ":"));
        if (seq_recv == seq_num_seen - 1) {
            // Deliver message
            seq_num_seen++;
            message_chat_queue_ready.push(msg_recv);
        } else {
            // Suspend delivery of message
            std::pair<int, std::string> _pair(seq_recv, msg_recv);
            message_chat_queue.push(_pair);
        }
        
    } else if (strcmp(msg_type, D_JOINREQ) == 0) {
        
        if (memberNode->leaderAddr == nullptr) { // I am the leader
            // send D_JOINLIST:ip1:port1:...
            // First need to add this member to the list (should not exist)
            addMember(fromAddr.getAddress(), false);
            std::string message = std::string(D_JOINLIST) + ":" + memberNode->getMemberList();
            dNet->DNsend(&fromAddr, message);
        } else {
            // send leader address
            std::string message = std::string(D_JOINLEADER) + ":" + memberNode->getLeaderAddress();
            dNet->DNsend(&fromAddr, message);
        }
        
    } else {
        std::cout << "Fail : recvHandler" << std::endl;
    }
    
    delete [] cstr;
}

/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				member: start election; leader: multicast LEAVE
 */
void DNode::nodeLoopOps() {
    // have the leader broadcast (members send) a heartbeat every x sec
    sendHeartbeat();
    
    // check a heartbeatList every x sec: for both leader and members
    // leader: heartbeatist would be a hashmap<, every time it recved a msg, update corresboding entry<MemberEntry, time_t>, with key = member address and time_t being last time it recved a msg or a heartbeat
    // member: keep a field for leader heartbest (last time it recved anything from the leader
    
    // if any one heartbeat in the list is (2x) sec away from the current system time
    
    // leader: broadcast its leave (once a recvHandler recv a LEAVE message, it will update the membershipList accordingly
    
    // member: start a election
    
    // finish heartbeat check
    return;
}

/**
 * FUNCTION NAME: sendHeartbeat
 *
 * DESCRIPTION: Send a heartbeat to the leader (called by members) 
 *              Or Send a multicast heartbeat to members (called by the leader)
 *              If the current node is the leader, call multicast directly
 *              There is a thread continuing calling this method
 */
void DNode::sendHeartbeat() {
    std::string leader_address = memberNode->getLeaderAddress();
    std::string self_address = memberNode->address->getAddress();
    if (leader_address.compare(self_address)) { // I'm the leader (sequencer)
        //multicast heartbeat to every one
        multicastHeartbeat();
    } else { // send heartbeat to leader
        std::stringstream ss;
        ss << HEARTBEAT;
        Address leader_addr(leader_address);
        dNet->DNsend(&leader_addr, ss.str());
#ifdef DEBUGLOG
        std::cout << "Send heartbeat to (Leader): " << leader_addr.getAddress() << std::endl;
#endif
    }
}

/**
 * FUNCTION NAME: multicastHeartbeat (maybe optimize into multipcastMsg later)
 *
 * DESCRIPTION: Multicast a heartbeat to all the members
 *              TODO: only leaders can call this function
 */
void DNode::multicastHeartbeat() {
    std::stringstream ss;
    auto list = memberNode->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        ss.clear();
        ss << HEARTBEAT;
        Address addr((*iter).getAddress());
        dNet->DNsend(&addr, ss.str());
#ifdef DEBUGLOG
        std::cout << "Multicast heartbeat to: " << addr.getAddress() << std::endl;
#endif
    }
}
