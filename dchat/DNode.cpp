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
        dNet->DNrecv(addr, data, -1);
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
    //    memberNode->heartbeat = 0;
    //    memberNode->pingCounter = TFAIL;
    //    memberNode->timeOutCounter = -1;
    memberNode->memberList.clear();
    return SUCCESS;
}


/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int DNode::introduceSelfToGroup(Address * joinaddr, bool isSureLeaderAddr) {
    std::string self_addr = memberNode->address->getAddress();
    std::string join_addr = joinaddr->getAddress();
    
    if (self_addr.compare(join_addr) == 0) {
        
        // I am the group booter (first process to join the group). Boot up the group
        std::cout << username << " started a new chat, listening on ";
        std::cout << memberNode->getAddress() << std::endl;
        addMember(join_addr, username, true); // I'm the leader
        
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
        if (dNet->DNrecv(address, message, -1) != SUCCESS) return FAILURE;
        
        // send JOINREQ message to introducer member
        size_t index = message.find(":");
        std::string msg_type = message.substr(0, index);
        
        if (msg_type.compare(D_JOINLEADER) == 0) {
            
            if (isSureLeaderAddr) return FAILURE; // Leader should return list
            std::string ip_port = message.substr(index + 1);
            memberNode->leaderAddr = new Address(ip_port);
            return this->introduceSelfToGroup(memberNode->leaderAddr, true); // Connect to leader
            
        } else if (msg_type.compare(D_JOINLIST) == 0) {
            
            std::string recv_param = message.substr(index + 1);
            std::string recv_init_seq = recv_param.substr(0, recv_param.find(":"));
            std::string recv_member_list = recv_param.substr(recv_param.find(":") + 1);
            
            multicast_queue = new holdback_queue(atoi(recv_init_seq.c_str()));
            initMemberList(recv_member_list, address.getAddress());
            
        } else {
            return FAILURE;
        }
    }
    
    memberNode->inGroup = true;
    return SUCCESS;
}

/**
 * FUNCTION NAME: initMemberList
 *
 * DESCRIPTION: initialize member list given member_list like the following
 *              ip1:port1:name1:ip2:port2:name2: ...
 */
void DNode::initMemberList(std::string member_list, std::string leaderAddr) {
    
    if (member_list.empty()) return;
    char * cstr = new char[member_list.length() + 1];
    std::string addr;
    strcpy(cstr, member_list.c_str());
    std::string ip(strtok(cstr, ":"));
    std::string port(strtok(NULL, ":"));
    std::string name(strtok(NULL, ":"));
    addr = ip + ":" + port;
    addMember(addr, name, addr.compare(leaderAddr) == 0);
    char * pch;
    while ((pch = strtok(NULL, ":")) != NULL) {
        ip = std::string(pch);
        port = std::string(strtok(NULL, ":"));
        name = std::string(strtok(NULL, ":"));
        addMember(ip + ":" + port, name, addr.compare(leaderAddr) == 0);
    }
    
}

/**
 * FUNCTION NAME: addMember
 *
 * DESCRIPTION:
 */
void DNode::addMember(std::string ip_port, std::string name, bool isLeader){
#ifdef DEBUGLOG
    std::cout << "DNode::addMember: " << ip_port << std::endl;
#endif
    MemberListEntry entry(ip_port, name); // memberList with no such boolean?
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
        std::string new_msg = std::string(D_MSG) + ":" + msg;
        multicastMsg(new_msg);
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
        ss << D_MULTI << ":" << seq << ":" << msg;
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
 *
 * MESSAGE:     CHAT : "Bob" : "Hello"
 *              MULTI : Seq : MSG :  "Bob:: Hello"
 *              MULTI : Seq : ADDNODE :" 0.0.0.0:8888:name"
 *
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
        
        std::string recv_name(strtok (NULL, ":"));
        std::string recv_msg(strtok (NULL, ":"));
        
        if (memberNode->leaderAddr == nullptr) { // Only leader can multicast messages
            std::string message = std::string(D_MSG) + ":" + recv_name + ":: " + recv_msg;
            multicastMsg(message);
        }
        
    } else if (strcmp(msg_type, D_MULTI)) {
        
        // MULTI : Seq : MSG :  "Bob:: Hello"
        int recv_seq = atoi(strtok (NULL, ":"));
        std::string recv_msg(strtok (NULL, ":"));
        multicast_queue->push(std::make_pair(recv_seq, recv_msg));
        
        if (recv_seq == seq_num_seen - 1) {
            seq_num_seen++;
            
        }
        
    } else if (strcmp(msg_type, D_JOINREQ) == 0) {
        
        if (memberNode->leaderAddr == nullptr) { // I am the leader
            // send D_JOINLIST:ip1:port1:name1:...
            // First need to add this member to the list (should not exist)
            // TODO : content is the username
            addMember(fromAddr.getAddress(), content, false);
            std::string message = std::string(D_JOINLIST) + ":" + memberNode->getMemberList();
            dNet->DNsend(&fromAddr, message);
            // Multicast addnode message
            // TODO: what if this message is lost - this message must be delivered once (at least onece)
            std::string message_addmember = std::string(D_ADDNODE) + ":" + fromAddr.getAddress();
            multicastMsg(message_addmember);
        } else {
            // send leader address
            std::string message = std::string(D_JOINLEADER) + ":" + memberNode->getLeaderAddress();
            dNet->DNsend(&fromAddr, message);
        }
        
    } else if (strcmp(msg_type, D_HEARTBEAT) == 0) {
        //update list for now, with fromAddr check
        time_t current;
        time(&current);
        memberNode->updateHeartBeat(fromAddr.getAddress(), current);
    } else if (strcmp(msg_type, D_LEAVE) == 0) {
        // TODO update member list
        
        // TODO push to message queue for display
    }else {
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
    std::string leader_address = memberNode->getLeaderAddress();
    std::string self_address = memberNode->address->getAddress();
    if(leader_address.compare(self_address) == 0) { // I am the leader
        // have the leader broadcast a heartbeat
        multicastHeartbeat();
        
        // check every one's heartbeat in the memberlist (except myself)
        auto list = memberNode->memberList;
        auto heartBook = memberNode->heartBeatList;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            std::string memberAddr = iter->getAddress();
            if(!memberAddr.compare(self_address)) {
                //check heartbeat
                time_t current;
                time(&current);
                time_t heartbeat = memberNode->getHeartBeat(memberAddr);
                if(difftime(current, heartbeat) > TIMEOUT/1000) {
                    //exceed timeout limit
                    std::string message_leave = std::string(D_LEAVE) + ":" + iter->username + ":" + memberAddr;
                    multicastMsg(message_leave);
                }
            }
        }
        
        
    } else { // I am a member
        // send a heart beat to the leader
        sendHeartbeat();
        
        // check leader heartbeat
        time_t current;
        time(&current);
        time_t heartbeat_ledaer = memberNode->getHeartBeat(leader_address);
        if(difftime(current, heartbeat_ledaer) > TIMEOUT/1000) {
            //TODO startElection();
        }
    }
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
    // send heartbeat to leader
    Address leader_addr(leader_address);
    dNet->DNsend(&leader_addr, D_HEARTBEAT);
#ifdef DEBUGLOG
    std::cout << "Send heartbeat to (Leader): " << leader_addr.getAddress() << std::endl;
#endif
    
}

/**
 * FUNCTION NAME: multicastHeartbeat (maybe optimize into multipcastMsg later)
 *
 * DESCRIPTION: Multicast a heartbeat to all the members
 *              TODO: only leaders can call this function
 */
void DNode::multicastHeartbeat() {
    auto list = memberNode->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        Address addr((*iter).getAddress());
        dNet->DNsend(&addr, D_HEARTBEAT);
#ifdef DEBUGLOG
        std::cout << "Multicast heartbeat to: " << addr.getAddress() << std::endl;
#endif
    }
}