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
        //if multi-cast message, msg_type==""
        msg_type = strtok(NULL, "#");
        if (strcmp(msg_type, D_M_ADDNODE) == 0) {
            // TODO: received: #ADDNODE#SEQ#ip#port#name
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_ip(strtok(NULL, "#"));
            std::string param_port(strtok(NULL, "#"));
            std::string param_name(strtok(NULL, "#"));
            node->addMember(param_ip, param_name, false);
        } else if (strcmp(msg_type, D_M_MSG)) {
            // TODO: received: #MSG#SEQ#Message
            std::string param_ip = strtok(NULL, "#");
            std::string param_port = strtok(NULL, "#");
            std::string param_name = strtok(NULL, "#");
            
        }

    } else {

        if (recv_msg.compare(D_CHAT) == 0) {
            // received: CHAT#Name#Message - From node to sequencer
            std::string recv_name(strtok (cstr, "#"));
            std::string recv_msg(strtok (NULL, "#"));
            
            if (nodeMember->leaderAddr == nullptr) { // Only leader can multicast messages
                // I am the leader
                // msg to be sent: #MSG#SEQ#Message
                std::string message = "#" + std::string(D_M_MSG) + "#" +
                std::to_string(node->getSeqNum()) + "#" + recv_msg;
                node->multicastMsg(message);
            }
        } else if (recv_msg.compare(D_JOINREQ) == 0) {
            // received: JOINREQ#PORT
            if (nodeMember->leaderAddr == nullptr) { // I am the leader
                // send JOINLIST#initSeq#ip1:port1:name1:ip2:port2:name2...
                // First need to add this member to the list (should not exist)
                // TODO: need initSeq
                node->addMember(from_addr.getAddress(), nodeUsername, false);
                std::string message = std::string(D_JOINLIST) + "#" + nodeMember->getMemberList();
                std::string str_ack;
                nodeDNet->DNsend(&from_addr, message, str_ack, 3);
                // Multicast addnode message
                // #ADDNODE#SEQ#ip#port#name, multicast from the sequencer
                // TODO: what if this message is lost - this message must be delivered once (at least onece)
                std::string message_addmember = "#" + std::string(D_M_ADDNODE) + "#" +
                std::to_string(node->getSeqNum())+ from_addr.getAddressIp() +
                "#" + from_addr.getAddressPort() + "#" + nodeUsername;
                node->multicastMsg(message_addmember);
            }
            else {
                // send leader address
                // received: JOINLEADER#LEADERIP#LEADERPORT
                std::string message = std::string(D_JOINLEADER) + "#" + nodeMember->getLeaderAddressIp()
                                    + "#" + nodeMember->getLeaderAddressPort();
                std::string str_ack;
                nodeDNet->DNsend(&from_addr, message, str_ack, 3);
            }
        } else if (recv_msg.compare(D_JOINLEADER)) {
            //TODO: received: JOINLEADER#LEADERIP#LEADERPORT
        } else if (recv_msg.compare(D_JOINLIST)) {
            //TODO: received: JOINLIST#initSeq#ip1:port1:name1:ip2:port2:name2...
        } else if (recv_msg.compare(D_LEAVE)) {
            //TODO: received: 
        } else if (recv_msg.compare(D_HEARTBEAT)) {
            
        }
        
    }
    
    
    
    
    
    
    return "";
}