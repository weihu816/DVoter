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
    std::string process(Address from_addr, std::string msg);
};

#endif /* HANDLER_H */
