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
    member_node = new Member(my_addr);
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
        std::cout << "init node failed. Exit." << std::endl;
        return FAILURE;
    }
    if( introduceSelfToGroup(join_address, false) == FAILURE ) {
        std::cout << "Sorry no chat is active on " << member_node->getAddress()
        << ", try again later.\nBye." << std::endl;
        return FAILURE;
    }
    std::cout << "Succeed, current users:" << std::endl; // TODO
    member_node->printMemberList();
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
    member_node->nnb = 0;
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
    
#ifdef DEBUGLOG
    std::cout << "\tDNode::initMemberList: " << member_list  << " leaderAddr:" << leaderAddr << std::endl;
#endif

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
        
        std::cout << username << " started a new chat, listening on " << member_node->getAddress() << std::endl;
        addMember(join_addr, username, true);
        m_queue = new holdback_queue(0, this);
        
    } else {
        
        Address to_addr(join_addr);
        // Requst to join by contacting the member
        std::string msg_to = std::string(D_JOINREQ) + "#" + std::to_string(member_node->address->port) + "#" + username;
        std::string msg_ack;
        if (dNet->DNsend(&to_addr, msg_to, msg_ack, 3) != SUCCESS) return FAILURE;
        
        // send JOINREQ message to introducer member
        size_t index = msg_ack.find("#");
        if (index == std::string::npos) return FAILURE;
        std::string msg_type = msg_ack.substr(0, index);
        
        if (msg_type.compare(D_JOINLIST) == 0) {
            
            // received: JOINLIST#initSeq#leadername#ip1:port1:name1:ip2:port2:name2...
            std::string recv_param = msg_ack.substr(index + 1);
            std::string recv_init_seq = recv_param.substr(0, recv_param.find("#"));
            std::string recv_name_list = recv_param.substr(recv_param.find("#") + 1);
            std::string recv_name = recv_name_list.substr(0, recv_name_list.find("#"));
            std::string recv_list = recv_name_list.substr(recv_name_list.find("#") + 1);
            m_queue = new holdback_queue(atoi(recv_init_seq.c_str()), this);
            initMemberList(recv_list, to_addr.getAddress());
            if (!isSureLeaderAddr) member_node->leaderEntry = new MemberListEntry(join_addr, recv_name);
            
        } else {
            std::cout << "(Never here) Bad behaviour in introduceSelfToGroup" << std::endl;
            return FAILURE;
        }
        
    }
  
    return SUCCESS;
}

void DNode::addMessage(std::string msg) {
    message_chat_queue.push(msg);
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
    

    
}



/**
 * FUNCTION NAME: multicastMsg
 *
 * DESCRIPTION: Multicast the message to all the members
 *              TODO: only leaders can call this function
 */
void DNode::multicastMsg(std::string msg, std::string type) {
        
}


/**
 * FUNCTION NAME: sendNotice (send non-chat, notice message without sequence num)
 *
 * DESCRIPTION: Send a single notice
 */
int DNode::sendNotice(std::string notice, std::string destAddress) {
    Address addr(destAddress);
    std::string str_ack;
    
    if (dNet->DNsend(&addr, notice, str_ack, 1) == FAILURE) {
        
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

///**
// * FUNCTION NAME: electionHandler
// *
// * DESCRIPTION: Handle the case if the potential fails during the election process
// */
//void electionHandler(DNode * node) {
//    // waits some time for D_COOR
//    std::cout << "electionHandler~~" << std::endl;
//    if (node->getElectionStatus() == E_WAITCOOR) {
//        std::chrono::milliseconds sleepTime(ELECTIONTIME);
//        std::this_thread::sleep_for(sleepTime);
//        
//        if(node->getElectionStatus() == E_WAITCOOR) {
//            node->updateElectionStatus(E_NONE);
//            node->startElection();
//        }
//    }
//}



///////////////////////////////////// HEARTBEAT FUNC /////////////////////////////////////


/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				member: start election; leader: multicast LEAVEANNO
 */
void DNode::nodeLoopOps() {
   
    // finish heartbeat check
    return;
}


/**
 * Check heart beat of a given address
 * Return SUCCESS if the node is alive and Failure otherwise
 */
int DNode::checkHeartbeat(std::string address) {
    time_t current;
    time(&current);
    time_t heartbeat = member_node->getHeartBeat(address);
    if(difftime(current, heartbeat) > TIMEOUT / 1000) {
        std::cout << address << " !!! " << difftime(current, heartbeat) << std::endl;
        return FAILURE;
    }
    return SUCCESS;
}


//////////////////////////////// GETTERS ////////////////////////////////

/**
 * dNet getter
 */
DNet * DNode::getDNet() {
    return dNet;
}

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