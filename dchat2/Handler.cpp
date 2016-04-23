/**********************************
 * FILE NAME: Handler.cpp
 *
 * DESCRIPTION: Handler class
 **********************************/

#include "Handler.h"
#include "DNode.h"

std::string Handler::process(Address & from_addr, std::string recv_msg) {
    
#ifdef DEBUGLOG
    std::cout << "\tHandling message: " + recv_msg << " from: " << from_addr.getAddress() << std::endl;
#endif
    
    std::string send_msg;
    if (recv_msg.empty()) return "";
    
    char cstr[MAXBUFLEN];
    strcpy(cstr, recv_msg.c_str());
    char * msg_type = strtok(cstr, "#");
    
#ifdef DEBUGLOG
    std::cout << "\tMessage Type: " << msg_type << std::endl;
#endif
    
    Member * nodeMember = node->getMember();

    if (strcmp(msg_type, D_PROPOSE) == 0) {
        
        // received: CHAT#Message - From node to sequencer
        std::string content = recv_msg.substr(recv_msg.find("#") + 1);
        
        // Get the id for this message
        int propose = std::max(node->A.first, node->P);
        // should include its own process id
        node->P = propose + 1;
        
        // Buffer this message
        queue_object q_object(propose, content);
        
        // really need a queue?
        node->queue->push(q_object);

        // Send Propose
        std::cout << "\t[INFO] Propose: " << propose << " pid: " << ::getpid() << std::endl;
        return std::to_string(propose) + "#" + std::to_string(::getpid());
        
    } else if (strcmp(msg_type, D_JOINREQ) == 0) {
        
        std::cout << "\t" <<  D_JOINREQ << std::endl;
        // received: JOINREQ#PORT#name
        // First need to add this member to the list (should not exist)
        // If it's a multi-threaded server, seq number should be sync with other message handling
        std::string recv_port(strtok (NULL, "#"));
        std::string recv_name(strtok (NULL, "#"));
        
        // #ADDNODE#SEQ#ip#port#name, multicast addnode message from the sequencer
        // This message must be delivered once (at least onece)
        std::string message_addmember = std::string(D_M_ADDNODE) + "#" + from_addr.getAddressIp() + "#" + recv_port + "#" + recv_name;
        
        // send JOINLIST#ip1:port1:name1:ip2:port2:name2...
        // After ack this message the newly joined node need to add self to the memberlist
        std::string message = std::string(D_JOINLIST) + "#" + nodeMember->getMemberList();
        
        // Start a thread to do the multicast to avoid blocking self
        std::thread thread(&DNode::multicast, node, message_addmember);
        thread.detach();
        // The following is handled by the above multicast
        // std::string member_addr = from_addr.getAddressIp() + ":" + recv_port;
        // std::string member_name = recv_name;
        // node->addMember(member_addr, member_name, false);

#ifdef DEBUGLOG
        std::cout << "\tHandling Returns: " << message << std::endl;
#endif

        return message;
        
    } else if (strcmp(msg_type, D_COMMIT) == 0) {
        
        // COMMIT#id#pid#oldid
        int param_id = std::stoi(std::string(strtok (NULL, "#")));
        int param_pid = std::stoi(std::string(strtok (NULL, "#")));
        int param_key = std::stoi(std::string(strtok (NULL, "#")));        
        if (param_id > node->A.first) {
            node->A.first = param_id;
            node->A.second = param_pid;
        } if (param_id > node->A.first) {
            node->A.second = std::max(node->A.second, param_pid);
        }
        node->queue->makeDeliverable(param_key);

    } else {

#ifdef DEBUGLOG
        std::cout << "\tReceive Unexpected: " << recv_msg << std::endl;
#endif
        
    }
    
    return "OK";
}