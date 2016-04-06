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
    std::cout << "\t" + username << " join an existing chat, listening on "
    << member_node->getAddress() << std::endl;
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
    if( initThisNode() == FAILURE ) {
        std::cout << "init_thisnode failed. Exit." << std::endl;
        return FAILURE;
    }
    if( introduceSelfToGroup(join_address, false) == FAILURE ) {
        finishUpThisNode();
        // TODO
        std::cout << "Sorry no chat is active on " << member_node->getAddress()
        << ", try again later." << std::endl;
        return FAILURE;
    }
    std::cout << "Succeed, current users:" << std::endl; // TODO
    //    printMembers();
    std::cout << username << " " << member_node->getLeaderAddress() << " (Leader)" << std::endl;
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
    member_node->nnb = 0;
    member_node->memberList.clear();
    return SUCCESS;
}


/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int DNode::finishUpThisNode() {
    
    return 0;
}



//////////////////////////////// MEMBER LIST UPDATE FUNC ////////////////////////////////

/**
 * FUNCTION NAME: initMemberList
 *
 * DESCRIPTION: initialize member list given member_list like the following
 *              ip1:port1:name1:ip2:port2:name2: ...
 */
void DNode::initMemberList(std::string member_list, std::string leaderAddr) {
    
#ifdef DEBUGLOG
    std::cout << "\tDNode::initMemberList: " << member_list  << " leaderAddr:" << leaderAddr << std::endl;
#endif

    if (member_list.empty()) return;
    char * cstr = new char[member_list.length() + 1];
    std::string addr;
    strcpy(cstr, member_list.c_str());
    std::string ip(strtok(cstr, "#"));
    std::string port(strtok(NULL, "#"));
    std::string name(strtok(NULL, "#"));
    addr = ip + "#" + port;
    addMember(addr, name, addr.compare(leaderAddr) == 0);
    char * pch;
    while ((pch = strtok(NULL, "#")) != NULL) {
        ip = std::string(pch);
        port = std::string(strtok(NULL, "#"));
        name = std::string(strtok(NULL, "#"));
        addMember(ip + "#" + port, name, addr.compare(leaderAddr) == 0);
    }
    
}

/**
 * FUNCTION NAME: addMember
 *
 * DESCRIPTION:
 */
void DNode::addMember(std::string ip_port, std::string name, bool isLeader){
    
#ifdef DEBUGLOG
    std::cout << "\tDNode::addMember: " << ip_port  << " " << name << std::endl;
#endif

    member_node->addMember(ip_port, name);
    if (isLeader) {
        member_node->leaderEntry = new MemberListEntry(ip_port, name);
    }
}

/**
 * FUNCTION NAME: deleteMember //or done by index? TODO
 *
 * DESCRIPTION: delete member from the memberList
 */
void DNode::deleteMember(MemberListEntry toRemove){
#ifdef DEBUGLOG
    std::cout << "DNode::deleteMember: " << toRemove.username << std::endl;
#endif
    member_node->deleteMember(toRemove);
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
    std::cout << "DNode::introduceSelfToGroup: " << self_addr << " (self) vs " << join_addr << std::endl;
#endif

    if (self_addr.compare(join_addr) == 0) {
        
        // I am the group booter (first process to join the group). Boot up the group
        std::cout << username << " started a new chat, listening on ";
        std::cout << member_node->getAddress() << std::endl;
        addMember(join_addr, username, true); // I'm the leader
        
        m_queue = new holdback_queue(0);
        
    } else {

        Address to_addr(join_addr);
        std::cout << username << " joining a new chat on " << join_addr << ", listening on ";
        std::cout << member_node->getAddress() << std::endl;
        // Requst to join by contacting the member
        std::string msg_to = std::string(D_JOINREQ) + "#" + std::to_string(to_addr.port) + "#" + username;
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
            
            // received: JOINLIST#initSeq#ip1:port1:name1:ip2:port2:name2...
            std::string recv_param = msg_ack.substr(index + 1);
            std::string recv_init_seq = recv_param.substr(0, recv_param.find("#"));
            std::string recv_member_list = recv_param.substr(recv_param.find("#") + 1);
            m_queue = new holdback_queue(atoi(recv_init_seq.c_str()));
            initMemberList(recv_member_list, to_addr.getAddress());
            
        } else {
            return FAILURE;
        }
    }
    
    member_node->inGroup = true;
    return SUCCESS;
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
    if (leader_address.compare(self_address)) { // I'm the leader (sequencer)
        std::string new_msg = std::string(D_M_MSG) + "#" + msg;
        multicastMsg(new_msg, D_M_MSG);
    } else { // Send Multicast request to the sequencer
        std::string str_to = std::string(D_CHAT) + "#" + username + "#" + msg;
        std::string str_ack;
        Address leader_addr(leader_address);
        dNet->DNsend(&leader_addr, str_to, str_ack, 3);
#ifdef DEBUGLOG
        std::cout << "\tSend message: " + str_to << " to (Leader): " << leader_addr.getAddress() << std::endl;
#endif
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
    std::cout << "\tmulticastMsg: seq: " << seq << " msg: " << msg << " " << type << std::endl;
#endif
    
    auto list = member_node->memberList;
    std::string send_msg =  "#" + type + "#" + std::to_string(seq) + "#" + msg;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        Address addr((*iter).getAddress());
        std::string ack;
        if (dNet->DNsend(&addr, send_msg, ack, 3) == FAILURE) {
#ifdef DEBUGLOG
            std::cout << "\tmulticastMsg: Fail! " << addr.getAddress() << std::endl;
#endif
            // Remove first then recursively multicastMsg
//            member_node->deleteMember(*iter);
//            multicastMsg(iter->getAddress(), D_LEAVEANNO);
        }
        
#ifdef DEBUGLOG
        std::cout << "\tMulticast: " << send_msg << " to: " << addr.getAddress() << std::endl;
#endif

    }
}


/**
 * FUNCTION NAME: sendNotice (send non-chat, notice message without sequence num)
 *
 * DESCRIPTION: Send a single notice
 */
void DNode::sendNotice(std::string notice, std::string destAddress) {
    Address addr(destAddress);
    std::string str_ack;
    dNet->DNsend(&addr, notice, str_ack, 1);
#ifdef DEBUGLOG
    std::cout << "Send notice to (leader): " << addr.getAddress() << std::endl;
#endif
    
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
        dNet->DNsend(&addr, notice, str_ack, 1);
#ifdef DEBUGLOG
        std::cout << "\tMulticast notice to: " << addr.getAddress() << std::endl;
#endif
    }
}


//////////////////////////////// LEADER ELECTION ////////////////////////////////


/**
 * FUNCTION NAME: startElection, called by a member
 *
 * DESCRIPTION: member start a election
 */
void DNode::startElection() {
    // send D_ELECTION to all other processes with higher IDs, expecting D_ANSWER
    auto list = member_node->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        if(iter->getAddress().compare(member_node->getAddress()) > 0) {
            sendNotice(D_ELECTION,iter->getAddress());
        }
    }
    
    // wait for some ANSWER, sleep? RECV in another thread so we are okay?
    election_status = E_WAITANS;
    std::chrono::milliseconds sleepTime(ELECTIONTIME);
    // if hears from no process with higher IDs, then it broadcasts D_COOR.
    if(election_status == E_WAITANS) {
        multicastNotice(D_COOR);
    }
}

/**
 * FUNCTION NAME: handle election related message
 *
 * DESCRIPTION: handle a election msg
 */
void DNode::handleElection(Address fromAddr, std::string type) {
//    if(type.compare(D_ELECTION) == 0) {
//        std::string heardFrom = fromAddr.getAddress();
//        // delete leader from member list TODO OOoOOOOOOoOOOOOO
//        // member_node->deleteMember(MemberListEntry entry)
//        
//        // display: leader (username) left the chat,  immediately? TODO
//        std::cout << " Leader Something Something Something MSG" << std::endl;
//        
//        // If hears D_ELECTION from a process with a higher ID,
//        if(fromAddr.getAddress().compare(member_node->getAddress()) > 0) {
//            // waits some time for D_COOR
//            election_status = E_WAITCOOR;
//            std::chrono::milliseconds sleepTime(ELECTIONTIME);
//            if(election_status == E_NONE) {// recv
//                // update the leader_list
//                member_node->leaderAddr = &fromAddr; // TODO : check
//            } else {
//                // If it does not receive this message in time, it re-broadcasts the D_ELECTION
//                startElection();
//            }
//        } else {
//            // If hears D_ELECTION from a process with a lower ID
//            // send back D_ANSWER and startElection myself
//            sendNotice(D_ANSWER, fromAddr.getAddress());
//            startElection();
//        }
//    } else if(type.compare(D_ANSWER) == 0) {
//        if(fromAddr.getAddress().compare(member_node->getAddress()) > 0 && election_status == E_WAITANS) {
//            election_status = E_WAITCOOR;
//        }
//    } else if(type.compare(D_COOR) == 0) {
//        if(election_status == E_WAITCOOR)
//        election_status = E_NONE;
//        // member_node->updateLeaderAddr(fromAddr);
//        // seqNum expected roll back
//    }

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
    std::string leader_address = member_node->getLeaderAddress();
    std::string self_address = member_node->address->getAddress();
    if(leader_address.compare(self_address) == 0) { // I am the leader
        // have the leader broadcast a heartbeat
        multicastNotice(D_HEARTBEAT);
        
        // check every one's heartbeat in the memberlist (except myself)
        auto list = member_node->memberList;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            std::string memberAddr = iter->getAddress();
            if(memberAddr.compare(self_address) != 0) {
                //check heartbeat
                if(checkHeartbeat(memberAddr) == FAILURE) {
                    //exceed timeout limit
                    std::string message_leave = memberAddr;
                    multicastMsg(message_leave, D_LEAVEANNO);
                }
            }
        }
    } else { // I am a member
        // send heartbeat to leader
        std::string leader_address = member_node->getLeaderAddress();
        sendNotice(D_HEARTBEAT, leader_address);
        // check leader heartbeat
        if(checkHeartbeat(leader_address) == FAILURE) {
            startElection();
        }
    }
    // finish heartbeat check
    return;
}

int DNode::checkHeartbeat(std::string address)
{
    time_t current;
    time(&current);
    time_t heartbeat = member_node->getHeartBeat(address);
    if(difftime(current, heartbeat) > TIMEOUT/1000) {
        return FAILURE;
    }
    return SUCCESS;
}

//////////////////////////////// GETTERS ////////////////////////////////

/***
 * dNet getter
 ***/
DNet * DNode::getDNet(){
    return dNet;
}

/***
 * member_node getter
 ***/
Member * DNode::getMember() {
    return member_node;
}

/***
 * username getter
 ***/
std::string DNode::getUsername() {
    return username;
}
