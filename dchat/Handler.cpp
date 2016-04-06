/**********************************
 * FILE NAME: Handler.cpp
 *
 * DESCRIPTION: Handler class
 **********************************/

#include "Handler.h"
#include "DNode.h"

using namespace std;

vector<string> splitstr(string ori, char deli) {
    vector<string> tmp;
    stringstream ss(ori);
    string tok;
    while (getline(ss, tok, deli)) {
        tmp.push_back(tok);
    }
    return tmp;
}

string Handler::process(Address from_addr, string recv_msg) {
    
#ifdef DEBUGLOG
    std::cout << "Handling message: " + recv_msg << " from: " << from_addr.getAddress() << std::endl;
#endif
    
    std::string send_msg;
    if (recv_msg.empty()) return "";
    
    char * cstr = new char[recv_msg.length() + 1];
    strcpy(cstr, recv_msg.c_str());
    char * msg_type = strtok(cstr, "#");
    
    Member * nodeMember = node->getMember();
    DNet * nodeDNet = node->getDNet();
    string nodeUsername = node->getUsername();
    
    if (recv_msg.find("#") == 0) { // Start with a #
        
        if (strcmp(msg_type, D_M_ADDNODE) == 0) {
            
            // received: #ADDNODE#SEQ#ip#port#name
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_ip(strtok(NULL, "#"));
            std::string param_port(strtok(NULL, "#"));
            std::string param_name(strtok(NULL, "#"));
            std::string param_key(msg_type);
            std::string param_value(param_ip + ":" + param_port + ":" + param_name);
            node->multicast_queue->push(std::make_pair(param_seq, param_value));
            
            return "OK";
            
        } else if (strcmp(msg_type, D_M_MSG) == 0) {
            
            // received: #MSG#SEQ#Message
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_value(strtok(NULL, "#"));
            node->multicast_queue->push(std::make_pair(param_seq, param_value));
            
            return "OK";
        } else if (recv_msg.compare(D_LEAVEANNO) == 0) { //# D_LEAVEANNO is leader broadcast member leaving
            // this needs to be in the queue and process in order of seq #
        
            // TODO: received: #LEAVEANNO#seq#ip:port sent by leader
            // node->startElection();
            // TODO: delete the member from the memberList and display message accrodingly.
            
            return "OK";
        }
        
    } else {
        
        if (recv_msg.compare(D_CHAT) == 0) {
            // received: CHAT#Name#Message - From node to sequencer
            std::string recv_name(strtok (NULL, "#"));
            std::string recv_msg(strtok (NULL, "#"));
            
            if (nodeMember->leaderAddr == nullptr) { // Only leader can multicast messages
                // I am the leader
                // msg to be sent: #MSG#SEQ#Message
                std::string message = "#" + std::string(D_M_MSG) + "#" +
                std::to_string(node->getSeqNum()) + "#" + recv_msg;
                node->multicastMsg(message, D_M_MSG);
            }
            return "OK";
            
        } else if (recv_msg.compare(D_JOINREQ) == 0) {
            // received: JOINREQ#PORT
            if (nodeMember->leaderAddr == nullptr) { // I am the leader
                // send JOINLIST#initSeq#ip1:port1:name1:ip2:port2:name2...
                // First need to add this member to the list (should not exist)
                int initSeq = node->multicast_queue->getSequenceSeen();
                node->addMember(from_addr.getAddress(), nodeUsername, false);
                
                std::string message = std::string(D_JOINLIST) + "#" + std::to_string(initSeq) +
                "#" + nodeMember->getMemberList();
                std::string str_ack;
                nodeDNet->DNsend(&from_addr, message, str_ack, 3);
                
                // Multicast addnode message
                // #ADDNODE#SEQ#ip#port#name, multicast from the sequencer
                // TODO: what if this message is lost - this message must be delivered once (at least onece)
                std::string message_addmember = "#" + std::string(D_M_ADDNODE) + "#" +
                std::to_string(node->getSeqNum())+ from_addr.getAddressIp() +
                "#" + from_addr.getAddressPort() + "#" + nodeUsername;
                
                node->multicastMsg(message_addmember, D_M_ADDNODE);
                return "OK";
                
            } else {
                // send leader address
                // received: JOINLEADER#LEADERIP#LEADERPORT
                std::string message = std::string(D_JOINLEADER) + "#" + nodeMember->getLeaderAddressIp()
                + "#" + nodeMember->getLeaderAddressPort();
                std::string str_ack;
                
                nodeDNet->DNsend(&from_addr, message, str_ack, 3);
                return "OK";
            }
        } else if (recv_msg.compare(D_JOINLEADER) == 0) {
            
            // received: JOINLEADER#LEADERIP#LEADERPORT
            // need to introduce self to leader
            std::string leaderIp(strtok(NULL, "#"));
            std::string leaderPort(strtok(NULL, "#"));
            Address leaderAddr = Address(leaderIp+":"+leaderPort);
           
            node->introduceSelfToGroup(&leaderAddr, true);
            return "OK";
            
        } else if (recv_msg.compare(D_JOINLIST) == 0) {
            // received: JOINLIST#initSeq#ip1:port1:name1:ip2:port2:name2...
            std::string memberList(strtok (NULL, "#"));
            node->initMemberList(memberList, nodeMember->getLeaderAddress());
            
        } else if (recv_msg.compare(D_LEAVE) == 0) {
            // TODO: received: LEAVE#name#ip:port
            
            std::string leave_name(strtok(NULL, "#"));
            std::string leave_addr(strtok(NULL, "#"));

            MemberListEntry leave_entry(leave_addr, leave_name);
            // delete leaving node from member_list
            node->deleteMember(leave_entry);
            // TODO: D_LEAVE is sent to the leader, and leader should send out LEAVEANNO
            return "OK";
            
        } else if (recv_msg.compare(D_HEARTBEAT) == 0) {
            
            // received: HEARTBEAT
            // know node at from_addr is still there, update heartbeat for node at from_addr
            std::string node_addr = from_addr.getAddress();
            time_t timev;
            time(&timev);
            nodeMember->updateHeartBeat(node_addr, timev);
            return "OK";
            
        } else if (recv_msg.compare(D_ELECTION) == 0) {
            
            // TODO: received: ELECTION (put it in a queue to be built soon)
            //node->handleElection(from_addr, D_ELECTION);
            return "OK";
            
        } else if (recv_msg.compare(D_ANSWER) == 0) {
            
            // TODO: received: ANSWER  (put it in a queue)
            //node->handleElection(from_addr, D_ANSWER);
            return "OK";
            
        } else if (recv_msg.compare(D_COOR) == 0) {
            
            // TODO: received: COOR (put it in a queue)
            //node->handleElection(from_addr, D_COOR);
            return "OK";
            
        }
        
    }

    return "";
}