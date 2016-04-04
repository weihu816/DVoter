#ifndef HANDLER_H
#define HANDLER_H

#include "stdincludes.h"
#include "DNode.h"

/**
 * CLASS NAME: Parser
 *
 * DESCRIPTION:
 */
class Handler {
private:
    DNode * node;
    
public:
    Handler(DNode * node) : node(node) {

    }
    std::string process(std::string msg);
};

#endif /* HANDLER_H */
