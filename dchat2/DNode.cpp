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
    queue = new blocking_priority_queue(this);
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
    if (initThisNode() == FAILURE) {
        std::cout << "init node failed. Exit." << std::endl;
        return FAILURE;
    }
    if (introduceSelfToGroup(join_address) == FAILURE) {
        std::cout << "Sorry no chat is active on " << join_address
        << ", try again later.\nBye." << std::endl;
        return FAILURE;
    }
    std::cout << "Succeed, current users:" << std::endl;
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
    std::cout << "\tDNode::initMemberList: " << member_list  << std::endl;
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
int DNode::introduceSelfToGroup(std::string join_addr) {
    
    std::string self_addr = member_node->getAddress();

#ifdef DEBUGLOG
    std::cout << "\tDNode::introduceSelfToGroup: " << self_addr << " (self) vs (join) " << join_addr << std::endl;
#endif
    
    if (self_addr.compare(join_addr) == 0) {
        
        addMember(join_addr, username, true);
        std::cout << username << " started a new chat, listening on " << member_node->getAddress() << std::endl;
        
        
    } else {
        
        addMember(self_addr, username, true);
        Address to_addr(join_addr);
        
        // Requst to join by contacting the member
        std::string msg_to = std::string(D_JOINREQ) + "#" + std::to_string(member_node->address->port) + "#" + username;
        std::string msg_ack;
        if (dNet->DNsend(&to_addr, msg_to, msg_ack, 1) != SUCCESS) return FAILURE;
        
        // send JOINREQ message to introducer member
        size_t index = msg_ack.find("#");
        if (index == std::string::npos) return FAILURE;
        std::string msg_type = msg_ack.substr(0, index);

        if (msg_type.compare(D_JOINLIST) == 0) {
            
            // received: JOINLIST#ip1:port1:name1:ip2:port2:name2...
            std::string param_list = msg_ack.substr(index + 1);
            initMemberList(param_list);

        } else {

            std::cout << "(Never here) Bad behaviour in introduceSelfToGroup" << std::endl;
            return FAILURE;
            
        }
        
    }
    
    return SUCCESS;
}

void DNode::addMessageToDisplay(std::string msg) {
    
    message_chat_queue.push(msg);

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
    
    multicast(std::string(D_M_CHAT) +  "#" + username + ":: " + msg);
    
}


/**
 * FUNCTION NAME: multicast (multicast non-chat message without sequence num)
 *
 * DESCRIPTION: Multicast a notice to all the members
 */
void DNode::multicast(std::string content) {

#ifdef INFOLOG
    std::cout << "[INFO] Multicast Begin " << std::endl;
#endif

    // msg is lie Bob:: Hello
    int max_id = -1;
    int max_pid = -1;
    auto list = member_node->getMemberEntryList();
    
    // Should not be no member
    if (list.size() == 0) {
        std::cout << "ERROR: no member (should not be possible)" << std::endl;
        return;
    }

    std::deque<int> old_ids;

    // 1st PROPOSE Phase
    std::string p1 = std::string(D_PROPOSE) + "#" + content;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        Address addr((*iter).getAddress());
#ifdef INFOLOG
        std::cout << "[INFO] Propose to: " << addr.getAddress() << std::endl;
#endif
        std::string str_propose;
        if (dNet->DNsend(&addr, p1, str_propose, 3) == SUCCESS) {
            
            // str_ack is the sequence number proposed
            int id = std::stoi(str_propose.substr(0, str_propose.find("#")));
            int pid = std::stoi(str_propose.substr(str_propose.find("#") + 1));
            old_ids.push_back(id);
#ifdef INFOLOG
            std::cout << "[INFO] Recv Propose: " << id << " pid: " << pid << " from: " << addr.getAddress() << std::endl;
#endif
            if (id > max_id) {
                max_id = id;
                max_pid = pid;
            } else if (id == max_id) {
                if (pid > max_pid) {
                    pid = max_pid;
                }
            }
            
        } else {
            
            // Failure handling
            std::cout << "(Multicast) Notice: " << (*iter).getUsername() << " left the chat or crashed" << std::endl;
            member_node->deleteMember((*iter).getAddress());
            return;
            
        }
    }
    
    // 2nd COMMIT Phase
#ifdef INFOLOG
    std::cout << "[INFO] COMMIT: max_id: " << max_id << " max_pid: " << max_pid << std::endl;
#endif
    std::string p2 = std::string(D_COMMIT) + "#" + std::to_string(max_id) + "#" + std::to_string(max_pid); // + "#" + old_id
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        Address addr((*iter).getAddress());
#ifdef INFOLOG
        std::cout << "[INFO] COMMIT to: " << addr.getAddress() << std::endl;
#endif
        
        std::string str_ack;
        int old_id = old_ids.front();
        old_ids.pop_front();
        if (dNet->DNsend(&addr, p2 + "#" + std::to_string(old_id), str_ack, 3) == SUCCESS) {
            
            
        
        } else {
            
            // Handle Failure : Resend?
        
        }
    }
    
}


///////////////////////////////////// HEARTBEAT FUNC /////////////////////////////////////


/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: send heart beats
 */
void DNode::nodeLoopOps() {

    // Finish heartbeat check
    auto list = member_node->getMemberEntryList();
    for (auto i = list.begin(); i != list.end(); i++) {
        Address addr((*i).getAddress());
        std::string data = std::string(D_HEARTBEAT) + "#" + member_node->address->getAddressPort() + "#" + (*i).getUsername();
        std::string ack;
        // Wait for ack and can retry for three times at maximum
        if (dNet->DNsend(&addr, data, ack, 3) == FAILURE) {
            // Node is failed
            std::cout << "Notice: " << (*i).getUsername() << " left the chat or crashed" << std::endl;
            member_node->deleteMember((*i).getAddress());
        }
    }
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
