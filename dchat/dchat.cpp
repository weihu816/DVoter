//
//  main.cpp
//  dchat
//
//  Created by weihu on 3/24/16.
//  Copyright © 2016 DVoter. All rights reserved.
//

#include "stdincludes.h"
#include "DNode.h"

std::string create();

/* Print out usage to stdout */
void usage(std::string msg);

//int main(int argc, const char * argv[]) {
//    // Check arguments
//    if (argc != 2 && argc != 3) {
//        usage("Incorrent number of arguments");
//        exit(EXIT_FAILURE);
//    }
//
//    if (argc == 2) { // Start a new chat group
//        std::string name(argv[1]);
//    } else { // Join an existing chat group
//        std::string name(argv[1]);
//        std::string addr_port(argv[2]);
//        size_t pos = addr_port.find(":");
//        if (pos == std::string::npos) {
//            usage("Error parsing ADDR:PORT");
//            exit(EXIT_FAILURE);
//        }
//        
////        char[] addr = addr_port.substr(0, pos).c_str();
////        char[] port = addr_port.substr(pos + 1, string::npos).c_str();
//    }
//    
//}

int main(int argc, const char * argv[]) {
//    DNode node();
//    std::cout << ans << std::endl;
//    DNode node;
//    node.sendMsg("hehehe");
}

void usage(std::string msg) {
    std::cout << msg << std::endl;
    std::cout << "Usage: $ ./dchat USER (start a chat)" << std::endl;
    std::cout << "       $ ./dchat USER ADDR:PORT (join a chat)" << std::endl;
}
