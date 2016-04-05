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

    if (recv_msg.find("#") == 0) { // MULTICAST

        char * chat_msg = strtok(cstr, "#");
        // Only leader can multicast messages
        node->multicastMsg(std::string(D_M_MSG) + "#" + std::string(chat_msg));

    } else {
        if (recv_msg.compare(D_CHAT) == 0) {
            
        } else if (recv_msg.compare(D_CHAT) == 0) {
            
        }
        
    }
    
    
    
    
    
    
    return "";
}