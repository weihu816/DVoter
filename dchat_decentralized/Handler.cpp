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


string Handler::process(Address & from_addr, string recv_msg) {
    
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
    
    if (recv_msg.find("#") == 0) { // Start with a #
        
      
        
    } else {
        
        if (strcmp(msg_type, D_CHAT) == 0) {

            // received: CHAT#Message - From node to sequencer
            std::string recv_msg(strtok (NULL, "#"));
            return "OK";
            
        } else {

#ifdef DEBUGLOG
            std::cout << "\tReceive Unexpected: " << recv_msg << std::endl;
#endif

        }

    }

    return "";
}