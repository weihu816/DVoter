/**********************************
 * FILE NAME: Util.cpp
 *
 * DESCRIPTION: 
 **********************************/

#include "stdincludes.h"
#include "DNode.h"

void checkMsg(DNode * node) {
    while (1) {
        if (std::cin.eof()) {
            // TODO
        }
        char msg[MAXBUFLEN];
        std::cin.getline(msg, MAXBUFLEN);
        if (strlen(msg) != 0) {
            node->sendMsg(std::string(msg));
        }
    }
}

