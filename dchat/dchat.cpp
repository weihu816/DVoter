/**********************************
 * FILE NAME: dchat.cpp
 *
 * DESCRIPTION:
 **********************************/

#include "stdincludes.h"
#include "DNode.h"

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: Print out usage to stdout
 */
void usage(std::string msg) {
    std::cout << msg << std::endl;
    std::cout << "Usage: $ ./dchat USER (start a chat)" << std::endl;
    std::cout << "       $ ./dchat USER ADDR:PORT (join a chat)" << std::endl;
}

void sendMsg(DNode * node) {
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

void recvMsg(DNode * node) {
    while (1) {
        std::string msg = node->recvMsg();
        std::cout << msg << std::endl;
    }
}

/**
 * FUNCTION NAME: main
 *
 * DESCRIPTION:
 */
int main(int argc, const char * argv[]) {
    // Check arguments
    if (argc != 2 && argc != 3) {
        usage("Incorrent number of arguments");
        exit(EXIT_FAILURE);
    }

    DNode *node;
    
    if (argc == 2) { // Start a new chat group
        std::string name(argv[1]);
        node = new DNode(name);
    } else { // Join an existing chat group
        std::string name(argv[1]);
        std::string ip_port(argv[2]);
        if (ip_port.find(":") == std::string::npos) {
            usage("Error parsing ADDR:PORT");
            exit(EXIT_FAILURE);
        }
        node = new DNode(name, ip_port);
    }

    // Node is up, introduced to the group
    if (!node->nodeStart()) {
        delete node;
        std::exit(1);
    }
    
    // Thread: Listening for input
    std::thread thread_sendMsg(sendMsg, node);
    // Thread: Receive chat messages
    std::thread thread_recvMsg(recvMsg, node);

    thread_sendMsg.join();
    thread_sendMsg.join();

    // Clean up and quit
    node->finishUpThisNode();
    delete node;
}
