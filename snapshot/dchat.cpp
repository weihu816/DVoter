/**********************************
 * FILE NAME: dchat.cpp
 *
 * DESCRIPTION:
 **********************************/

#include "stdincludes.h"
#include "DNode.h"

volatile bool isAlive = true;

/**
 * FUNCTION NAME: usage
 *
 * DESCRIPTION: Print out usage to stdout
 */
void usage(std::string msg) {
    std::cout << msg << std::endl;
    std::cout << "Usage: $ ./dchat USER (start a chat)" << std::endl;
    std::cout << "       $ ./dchat USER ADDR:PORT (join a chat)" << std::endl;
}

//////////////////////////////// THREAD FUNC ////////////////////////////////

/**
 * FUNCTION NAME: sendMsg
 * 
 * DESCRIPTION: thread keep listening to user input
 */
void sendMsg(DNode * node) {
    while (isAlive) {
        /* Simply exit */
        if (std::cin.eof()) { // Control-D / EOF: shutdown
            isAlive = false;
            node->nodeLeave();
            delete node;
            std::exit(0);
        }
        char msg[MAXBUFLEN];
        std::cin.getline(msg, MAXBUFLEN - 1);
        if (strlen(msg) != 0) {
            std::string str(msg);
            if (str.compare("SS") == 0) {
                node->startSnapshotByUser();  // SNAPSHOT
            }
            else {
                node->sendMsg(std::string(msg));
            }
        }
    }
}

void sendMsgToLeader(DNode * node) {
    while (isAlive) {
        node->sendMsgToLeader();
    }
}

/**
 * FUNCTION NAME: displayMsg
 *
 * DESCRIPTION: thread in charge of displaying messages
 */
void displayMsg(DNode * node) {
    while (isAlive) {
        std::string msg = node->msgLoop();
        std::cout << msg << std::endl;
    }
}


/**
 * FUNCTION NAME: recvMsg
 *
 * DESCRIPTION: thread keep listening to incoming messages
 */
void recvMsg(DNode * node) {
    while (isAlive) {
        node->recvLoop();
    }
}


/**
 * FUNCTION NAME: heartBeatRoutine
 *
 * DESCRIPTION: send and check heartbeat
 */
void heartBeatRoutine(DNode * node) {
    while (isAlive) {
        std::chrono::milliseconds sleepTime(HEARTFREQ); // check every HEARTFREQ seconds
        std::this_thread::sleep_for(sleepTime);
        node->nodeLoopOps();
    }
}

//////////////////////////////// MAIN: START ////////////////////////////////

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
    if (node->nodeStart() == FAILURE) {
        delete node;
        return 0;
    }
    
    // Thread: Listening for input
    std::thread thread_sendMsg(sendMsg, node);
    // Thread: Receive chat messages
    std::thread thread_recvMsg(recvMsg, node);
    // Thread: Receive chat messages
    std::thread thread_displayMsg(displayMsg, node);
    // Thread: Track heartbeat
    std::thread thread_heartbeat(heartBeatRoutine, node);
    
    std::thread thread_sendMsgToLeader(sendMsgToLeader, node);

    thread_sendMsg.join();
    thread_recvMsg.join();
    thread_displayMsg.join();
    thread_heartbeat.join();
    
    // Clean up and quit
    node->nodeLeave();
    delete node;
    return 0;
}
