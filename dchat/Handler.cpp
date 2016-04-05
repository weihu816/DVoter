/**********************************
 * FILE NAME: Parser.cpp
 *
 * DESCRIPTION: Parser class
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

    if (recv_msg.find("#") == 0) { // Start with a #
        
        if (strcmp(msg_type, D_M_ADDNODE) {

            // #MSG#SEQ#Message
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_ip(strtok(NULL, "#"));
            std::string param_port(strtok(NULL, "#"));
            std::string param_name(strtok(NULL, "#")):
            node->addMember(param_ip, param_name, false);

        } else if (strcmp(msg_type, D_M_msg)) {
            
            // #ADDNODE#SEQ#ip#port#name
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_msg(strtok(NULL, "#"));
            
            
        }

    } else {

        if (recv_msg.compare(D_CHAT) == 0) {
            // Only leader can multicast messages
            node->multicastMsg("#" + std::string(D_M_MSG) + "#" + std::string(chat_msg));
        } else if (recv_msg.compare(D_CHAT) == 0) {
            
        }
        
    }
    
    
    
    
    
    
    return "";
}