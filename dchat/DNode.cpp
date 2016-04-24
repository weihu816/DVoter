/**********************************
 * FILE NAME: DNode.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of DNode class functions.
 **********************************/

#include "DNode.h"
#include "Handler.h"

/**
 * CONSTRUCTOR
 */
DNode::DNode(std::string name, std::string join_addr) : username(name) {
    dNet = new DNet(new Handler(this));
    dNet->DNinit();
    std::string my_addr;
    dNet->DNinfo(my_addr);
    member_node = new Member(my_addr);        // Create Member node
    if (!join_addr.empty()) {
        join_address = join_addr;
    } else {
        join_address = my_addr;
    }
}

//////////////////////////////// THREAD FUNC ////////////////////////////////

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int DNode::recvLoop() {
    Address addr;
    std::string data;
    dNet->DNrecv(addr, data);
    return SUCCESS;
}

/**
 * FUNCTION NAME: msgLoop
 *
 * DESCRIPTION: This function receives chat messages and displays to terminal
 */
std::string DNode::msgLoop() {
#ifdef DEBUGLOG
    std::cout << "\tpop from message_chat_queue" << std::endl;
#endif
    return message_chat_queue.pop();
}

//////////////////////////////// NODE INIT AND CLEAN FUNC ////////////////////////////////


/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
int DNode::nodeStart() {
    // Self booting routines
    if (member_node->getAddress().compare(join_address) != 0) {
        std::cout << username << " joining a new chat on " << join_address;
        std::cout << ", listening on " << member_node->getAddress() << std::endl;
    }
    if( initThisNode() == FAILURE ) {
        std::cout << "init_thisnode failed. Exit." << std::endl;
        return FAILURE;
    }
    if( introduceSelfToGroup(join_address, false) == FAILURE ) {
        std::cout << "Sorry no chat is active on " << join_address
        << ", try again later.\nBye." << std::endl;
        return FAILURE;
    }
    std::cout << "Succeed, current users:" << std::endl; // TODO
    std::cout << member_node->getLeaderName() << " " << member_node->getLeaderAddress() << " (Leader)" << std::endl;
    member_node->printMemberList();
    if (member_node->isLeader()) std::cout << "Waiting for others to join" << std::endl;
    
    return SUCCESS;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int DNode::initThisNode() {
    member_node->inited = true;
    member_node->inGroup = false;
    member_node->memberList.clear();
    return SUCCESS;
}


/**
 * FUNCTION NAME: nodeLeave
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int DNode::nodeLeave() {
    return SUCCESS;
}



//////////////////////////////// MEMBER LIST UPDATE FUNC ////////////////////////////////

/**
 * FUNCTION NAME: initMemberList
 *
 * DESCRIPTION: initialize member list given member_list like the following
 *              ip1:port1:name1:ip2:port2:name2: ...
 */
void DNode::initMemberList(std::string member_list) {

    if (member_list.empty()) return;
    char * cstr = new char[member_list.length() + 1];
    std::string addr;
    strcpy(cstr, member_list.c_str());
    char * pch = strtok(cstr, "#");
    while (pch != NULL) {
        std::string entry(pch);
        MemberListEntry listEntry(entry);
        addMember(listEntry.getAddress(), listEntry.username, false);
        pch = strtok(NULL, "#");
    }
    
}

/**
 * FUNCTION NAME: addMember
 *
 * DESCRIPTION:
 */
void DNode::addMember(std::string ip_port, std::string name, bool toPrint){
    
#ifdef DEBUGLOG
    std::cout << "\tDNode::addMember: " << ip_port  << " " << name << std::endl;
#endif
    
    member_node->addMember(ip_port, name);
    if (toPrint) {
        std::cout << "NOTICE " << name << " joined on " << ip_port << std::endl;
    }
}


/**
 * FUNCTION NAME: deleteMember
 *
 * DESCRIPTION: delete member from the memberList
 */
void DNode::deleteMember(std::string memberAddr){
#ifdef DEBUGLOG
    std::cout << "DNode::deleteMember... " << std::endl;
#endif
    std::string memberName = member_node->deleteMember(memberAddr);
    if(!memberName.empty()) {
        std::cout << "NOTICE " << memberName << " left the chat or crashed" << std::endl;
    }
}



//////////////////////////////// CLIENT: OPERATIONS FUNC ////////////////////////////////

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int DNode::introduceSelfToGroup(std::string join_addr, bool isSureLeaderAddr) {
    
    std::string self_addr = member_node->getAddress();
    
#ifdef DEBUGLOG
    std::cout << "\tDNode::introduceSelfToGroup: " << self_addr << " (self) vs " << join_addr << std::endl;
#endif
    
    if (self_addr.compare(join_addr) == 0) {
        
        // I am the group booter (first process to join the group). Boot up the group
        std::cout << username << " started a new chat, listening on " << member_node->getAddress() << std::endl;
        // addMember(join_addr, username, true); // I'm the leader
        member_node->leaderEntry = new MemberListEntry(join_addr, username);
        
        m_queue = new holdback_queue(0, this);
        
    } else {
        
        Address to_addr(join_addr);
        // Requst to join by contacting the member
        std::string msg_to = std::string(D_JOINREQ) + "#" + std::to_string(member_node->address->port) + "#" + username;
        std::string msg_ack;
        if (dNet->DNsend(&to_addr, msg_to, msg_ack, 3) != SUCCESS) return FAILURE;
        // Receive member lists from the leader, then save the leader address
        
        // send JOINREQ message to introducer member
        size_t index = msg_ack.find("#");
        if (index == std::string::npos) return FAILURE;
        std::string msg_type = msg_ack.substr(0, index);
        
        if (msg_type.compare(D_JOINLEADER) == 0) {
            
            // received: JOINLEADER#LEADERIP#LEADERPORT#LEADERNAME
            if (isSureLeaderAddr) return FAILURE;
            std::string ip_port_name = msg_ack.substr(index + 1);
            std::string ip = ip_port_name.substr(0, ip_port_name.find("#"));
            std::string port_name = ip_port_name.substr(ip_port_name.find("#") + 1);
            std::string port = port_name.substr(0, port_name.find("#"));
            std::string name = port_name.substr(port_name.find("#") + 1);
            member_node->leaderEntry = new MemberListEntry(ip + ":" + port, name);
            return this->introduceSelfToGroup(member_node->getLeaderAddress(), true);
            
        } else if (msg_type.compare(D_JOINLIST) == 0) {
            
            // received: JOINLIST#initSeq#leadername#ip1:port1:name1:ip2:port2:name2...
            std::string recv_param = msg_ack.substr(index + 1);
            std::string recv_init_seq = recv_param.substr(0, recv_param.find("#"));
            std::string recv_name_list = recv_param.substr(recv_param.find("#") + 1);
            std::string recv_name = recv_name_list.substr(0, recv_name_list.find("#"));
            std::string recv_list = recv_name_list.substr(recv_name_list.find("#") + 1);
            m_queue = new holdback_queue(atoi(recv_init_seq.c_str()), this);
            initMemberList(recv_list);
            if (!isSureLeaderAddr) member_node->leaderEntry = new MemberListEntry(join_addr, recv_name);
            
        } else {
            return FAILURE;
        }
    }
    
    member_node->inGroup = true;
    return SUCCESS;
}

void DNode::addMessage(std::string msg) {
    message_chat_queue.push_back(msg);
#ifdef DEBUGLOG
    std::cout << "\tpush to message_chat_queue" << std::endl;
#endif
}


//////////////////////////////// SEND MSG FUNC ////////////////////////////////


/**
 * FUNCTION NAME: sendMsg
 *
 * DESCRIPTION: Send a chat message
 *              If the current node is the leader, call multicast directly
 *              There is a thread continuing calling this method
 */
void DNode::sendMsg(std::string msg) {
    
    std::string leader_address = member_node->getLeaderAddress();
    std::string self_address = member_node->address->getAddress();

    if (leader_address.compare(self_address) == 0) { // I'm the leader (sequencer)
        std::string msgToSend = username + ":: " + msg;
        multicastMsg(msgToSend, D_M_MSG);
    } else { // Send Multicast request to the sequencer
        message_table[seq] = msg;
        message_send_queue.push_back(std::make_pair(seq, msg));
        seq++;
    }
    
}

/**
 * FUNCTION NAME: sendMsgToLeader
 *
 * DESCRIPTION: If election in process Need to take care msgs and send to the new leader
 *              No request will be send to leader if election in process
 */
void DNode::sendMsgToLeader() {
    
    auto msg_pair = message_send_queue.pop();
    
    std::unique_lock<std::mutex> lk(mutex_election); // message with port
    // Wait for election to be finished
    while (getElectionStatus() != E_NONE) {
        d_condition.wait(lk);
    }

    if (member_node->getLeaderAddress().compare(member_node->getAddress()) == 0) {
        
        // I'm the leader now
        std::string msgToSend = username + ":: " + msg_pair.second;
        multicastMsg(msgToSend, D_M_MSG);
        
    } else {
        
        std::string str_to = std::string(D_CHAT) + "#" + std::to_string(member_node->address->port) +"#" + std::to_string(msg_pair.first) + "#"
            + username + ":: " + msg_pair.second;
        std::string leader_address = member_node->getLeaderAddress();
        std::string str_ack;
        Address leader_addr(leader_address);
        if (dNet->DNsend(&leader_addr, str_to, str_ack, 1) == SUCCESS) {
            
            // If the response is not OK, then it is a number that the leader current seen
            std::string ok("OK");
            if (strcmp(str_ack.c_str(), ok.c_str()) != 0) {
                std::cout << str_to << " ACK from leader: " << str_ack << std::endl;
                int ack = stoi(str_ack);
                // if the ack seq number is less seq - 1, we need to resend that message
                while (ack + 1 < seq) {
                    std::string resend_ack;
                    std::string resend_str = std::string(D_CHAT) + "#" + std::to_string(ack) + "#"
                    + username + ":: " + message_table[ack];
                    std::cout << "Resend " + resend_str << std::endl;
                    dNet->DNsend(&leader_addr, resend_str, resend_ack, 1);
                    ack++;
                }
            }
            
        } else {
            
            // We think the leader is not responding
            // However we are not sure if the leader has multicast this message or not
            message_send_queue.push_front(msg_pair);
            lk.unlock();
            std::chrono::milliseconds sleepTime(1000);
            std::this_thread::sleep_for(sleepTime);
            
        }
        
    }
    
}


/**
 * FUNCTION NAME: multicastMsg
 *
 * DESCRIPTION: Multicast the message to all the members
 *              TODO: only leaders can call this function
 */
void DNode::multicastMsg(std::string msg, std::string type) {
    
    int seq = m_queue->getNextSequence();
    
#ifdef DEBUGLOG
    std::cout << "\tmulticastMsg: seq: " << seq << " msg: " << msg << " type: " << type << std::endl;
#endif
    
    std::string send_msg =  "#" + type + "#" + std::to_string(seq) + "#" + msg;
    // Send to group members
    auto list = member_node->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        Address addr((*iter).getAddress());
        std::string ack;
        if (dNet->DNsend(&addr, send_msg, ack, 2) == FAILURE) {
#ifdef DEBUGLOG
            std::cout << "\tmulticastMsg: Fail! " << addr.getAddress() << std::endl;
#endif
            // Remove first then recursively multicastMsg
            member_node->deleteMember((*iter).getAddress());
            multicastMsg(iter->getAddress(), D_LEAVEANNO);
        }
#ifdef DEBUGLOG
        std::cout << "\tMulticast: " << send_msg << " to: " << addr.getAddress() << std::endl;
#endif
    }
    // Send to self
    m_queue->push(std::make_pair(seq, "#"+ type + "#" + msg));
    m_queue->pop();
}


/**
 * FUNCTION NAME: sendNotice (send non-chat, notice message without sequence num)
 *
 * DESCRIPTION: Send a single notice
 */
int DNode::sendNotice(std::string notice, std::string destAddress) {
    Address addr(destAddress);
    std::string str_ack;
    
    if (dNet->DNsend(&addr, notice, str_ack, 3) == FAILURE) {
        
#ifdef DEBUGLOG
        std::cout << "\tsendNotice: Fail! " << addr.getAddress() << std::endl;
#endif
        return FAILURE;
        
    } else {
        
#ifdef DEBUGLOG
        std::cout << "Send notice to: " << addr.getAddress() << std::endl;
#endif
        return SUCCESS;
        
    }
}


/**
 * FUNCTION NAME: multicastNotice (multicast non-chat message without sequence num)
 *
 * DESCRIPTION: Multicast a notice to all the members
 *              TODO: only leaders can call this function
 */
void DNode::multicastNotice(std::string notice) {
    
    auto list = member_node->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        Address addr((*iter).getAddress());
        std::string str_ack;
        if (dNet->DNsend(&addr, notice, str_ack, 2) == FAILURE) {
#ifdef DEBUGLOG
            std::cout << "\tSendNotice: Fail! " << addr.getAddress() << std::endl;
#endif
        }
        //#ifdef DEBUGLOG
        std::cout << "\tSend notice to: " << notice << " " << addr.getAddress() << std::endl;
        //#endif
    }
}


//////////////////////////////// LEADER ELECTION ////////////////////////////////

/**
 * FUNCTION NAME: startElection, called by a member
 *
 * DESCRIPTION: member start a election
 */
void DNode::startElection() {
    
    if(getElectionStatus() != E_NONE) {
        return; // In election already
    }
    
    std::unique_lock<std::mutex> lk(mutex_election);
    
    std::cout << ">>>>> Start Leader Election <<<<<" << std::endl;
    
    // wait for some ANSWER, sleep? RECV in another thread so we are okay?
    updateElectionStatus(E_WAITANS);
    
    // Send D_ELECTION to all other processes with higher IDs, expecting D_ANSWER
    auto list = member_node->memberList;
    int send_count = 0;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        if(iter->getAddress().compare(member_node->getAddress()) > 0) {
            int status = sendNotice(std::string(D_ELECTION) + "#" + member_node->getAddress(), iter->getAddress());
            std::cout << "Send Election to " << iter->getAddress() << " : " << status << std::endl;
            if (status == SUCCESS) {
                send_count++;
            }
        }
    }
    // Send Election to leader as well
    if (member_node->getLeaderAddress().compare(member_node->getAddress()) > 0) {
        int status = sendNotice(std::string(D_ELECTION) + "#" + member_node->getAddress(), member_node->getLeaderAddress());
        std::cout << "Send Election to " << member_node->getLeaderAddress() << " : " << status << std::endl;
        if (status == SUCCESS) {
            send_count++;
        }
    }
    
    // If it knows its id is the highest, it elect itself as coordinator
    // Then sends a Coordinator message to all processes with lower identifier. Election is complete
    if (send_count == 0) {
        updateElectionStatus(E_NONE);
        // Delete your own address from the member list
        member_node->deleteMember(member_node->getAddress());
        // Update leader address
        member_node->updateLeader(*(member_node->address), username);
        
        // However, once notice is sent the server is ready to receive message, so we lock it ???
        // Multicast COOR message to the rest of nodes
        multicastNotice(std::string(D_COOR) + "#" + username + "#" + member_node->getAddress());
        
        m_queue->resetSequence();
        resetSeq();
        
        return;
    }
    
    // Wait for COOR message ..........
    updateElectionStatus(E_WAITCOOR);
    lk.unlock();
    
    std::cout << "Wait for COOR message .........." << std::endl;
    std::chrono::milliseconds sleepTime(COORTIMEOUT);
    std::this_thread::sleep_for(sleepTime);
    
    if(getElectionStatus() == E_WAITCOOR) {
        std::cout << "No COOR message .........." << std::endl;
        updateElectionStatus(E_NONE);
        startElection();
    }
    
}


///////////////////////////////////// HEARTBEAT FUNC /////////////////////////////////////


/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				member: start election; leader: multicast LEAVEANNO
 */
void DNode::nodeLoopOps() {
    
    std::unique_lock<std::mutex> lk(mutex_election);
    
    std::string leader_address = member_node->getLeaderAddress();
    std::string self_address = member_node->address->getAddress();
    
    if(leader_address.compare(self_address) == 0) { // I am the leader
        
        // check every one's heartbeat in the memberlist (except myself)
        auto list = member_node->memberList;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            
            std::string memberAddr = iter->getAddress();
            if(memberAddr.compare(self_address) != 0) {
                if (sendNotice(std::string(D_HEARTBEAT) + "#" + self_address, memberAddr) == FAILURE) {
                    // exceed timeout limit
                    deleteMember(memberAddr);
                    // std::string message_leave = memberAddr;
                    multicastMsg(memberAddr, D_LEAVEANNO);
                }
            } else {
                std::cout << "!!! Never here in nodeLoopOps " << memberAddr << std::endl;
            }
        }
    } else {
        
        // I am a member and I send heartbeat to leader
        std::string leader_address = member_node->getLeaderAddress();
        std::string self_address = member_node->getAddress();
        
        if (sendNotice(std::string(D_HEARTBEAT) + "#" + self_address, leader_address) == FAILURE) {
#ifdef DEBUGLOG
            std::cout << "\tLeader failed. " << std::endl;
#endif
            
            // Must release the locak before start election
            lk.unlock();
            startElection();
            
        }
    }
    
    // finish heartbeat check
    return;
}


//////////////////////////////// GETTERS ////////////////////////////////
/**
 * member_node getter
 */
Member * DNode::getMember() {
    return member_node;
}

/**
 * username getter
 */
std::string DNode::getUsername() {
    return username;
}

/**
 * election status getter
 */
int DNode::getElectionStatus() {
    mtx.lock();
    int toReturn = election_status;
    mtx.unlock();
    return toReturn;
}

/**
 * update election status
 */
void DNode::updateElectionStatus(int new_status) {
    
    mtx.lock();
    election_status = new_status;
    mtx.unlock();
#ifdef DEBUGLOG
    std::cout << "\tElection status update: " << new_status << std::endl;
#endif
    
}