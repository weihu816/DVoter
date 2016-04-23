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
        int id = std::max(node->A, node->P);
        // should include its own process id
        node->P = std::max(node->A, node->P) + 1;
        
        // Buffer this message
        queue_object q_object(id, content);
        
        // really need a queue?
        node->queue.push(q_object);

        // id is increasing so ok for unique identifier
        node->buffer[id] = q_object;
        
        // Send Propose
        std::cout << "[INFO] Propose: " << id << " pid: " << ::getpid() << std::endl;
        return std::to_string(id) + "#" + std::to_string(::getpid());
        
    } else if (strcmp(msg_type, D_JOINREQ)) {
        
        // received: JOINREQ#PORT#name
        // First need to add this member to the list (should not exist)
        // If it's a multi-threaded server, seq number should be sync with other message handling
        std::string recv_port(strtok (NULL, "#"));
        std::string recv_name(strtok (NULL, "#"));
        
        // #ADDNODE#SEQ#ip#port#name, multicast addnode message from the sequencer
        // This message must be delivered once (at least onece)
        std::string message_addmember = std::string(D_M_ADDNODE) + "#" + from_addr.getAddressIp() + "#" + recv_port + "#" + recv_name;
        
        // Start a thread to do the multicast to avoid blocking self
        node->multicast(message_addmember);
        
        std::string member_addr = from_addr.getAddressIp() + ":" + recv_port;
        std::string member_name = recv_name;
        
        node->addMember(member_addr, member_name, false);
        
        // send JOINLIST#ip1:port1:name1:ip2:port2:name2...
        std::string message = std::string(D_JOINLIST) + "#" + node->getUsername() + "#" + nodeMember->getMemberList();
        
#ifdef DEBUGLOG
        std::cout << "\tHandling Returns: " << message << std::endl;
#endif
        
        return message;
        
    } else if (strcmp(msg_type, D_COMMIT)) {
        
        // COMMIT#id#pid
        
        std::string recv_seq(strtok (NULL, "#"));
        std::string recv_type(strtok (NULL, "#"));
        int seq = std::stoi(recv_seq);
        node->A = std::max(node->A, seq);

        if (strcmp(recv_type.c_str(), D_M_CHAT) == 0) {
            
            std::string recv_msg(strtok (NULL, "#"));
            std::vector<std::string> vec;
            vec.push_back(recv_msg);
            node->queue.push(queue_object(std::string(D_M_CHAT), vec));

        } else if (strcmp(recv_type.c_str(), D_M_ADDNODE) == 0) {

            std::string recv_ip(strtok (NULL, "#"));
            std::string recv_port(strtok (NULL, "#"));
            std::string recv_name(strtok (NULL, "#"));
            std::vector<std::string> vec;
            vec.push_back(recv_ip);
            vec.push_back(recv_port);
            vec.push_back(recv_name);
            node->queue.push(queue_object(std::string(D_M_ADDNODE), vec));

        }

    } else {
        
#ifdef DEBUGLOG
        std::cout << "\tReceive Unexpected: " << recv_msg << std::endl;
#endif
        
    }
    
    return "OK";
}