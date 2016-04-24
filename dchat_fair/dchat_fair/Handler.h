#ifndef HANDLER_H
#define HANDLER_H

#include "stdincludes.h"
#include "Member.h"

class DNode;

/**
 * CLASS NAME: Parser
 *
 * DESCRIPTION:
 */
class Handler {
private:
    DNode * node;
    
public:
    Handler(DNode * node) : node(node) { }
    std::string process(Address & from_addr, std::string msg);
    int amLeader();
    std::string getMemberAddrByIndex(int &tableCounter);
    int getMemberListLength();
    std::string getAddress();
    void multicast(std::string msg, std::string type);
};

#endif /* HANDLER_H */
