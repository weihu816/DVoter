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

/**
 * FUNCTION NAME: sendMsg
 * 
 * DESCRIPTION: thread keep listening to user input
 */
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

/**
 * FUNCTION NAME: recvMsg
 *
 * DESCRIPTION: thread keep listening to incoming messages
 */
void recvMsg(DNode * node) {
    while (1) {
        node->recvLoop();
    }
}

/**
 * FUNCTION NAME:
 *
 * DESCRIPTION:
 */
void handleMsg(DNode * node) {
    while (1) {
        node->nodeLoop();
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
        std::cout << "Fail to start the node" << std::endl;
        std::exit(1);
    }
    
    // Thread: Listening for input
    std::thread thread_sendMsg(sendMsg, node);
    // Thread: Receive chat messages
    std::thread thread_recvMsg(recvMsg, node);
    // Thread: Keep looking at message queue and handles message
    std::thread thread_handleMsg(handleMsg, node);

    thread_sendMsg.join();
    thread_recvMsg.join();
    thread_handleMsg.join();
    
    // Clean up and quit
    node->finishUpThisNode();
    delete node;
}
