//
//  main.cpp
//  dchat
//
//  Created by weihu on 3/24/16.
//  Copyright © 2016 DVoter. All rights reserved.
//

#include "stdincludes.h"

#include "User.h"

std::string create_addr();

/* Print out usage to stdout */
void usage(string msg);

int main(int argc, const char * argv[]) {
    // Check arguments
    if (argc != 2 && argc != 3) {
        usage("Incorrent number of arguments");
        exit(EXIT_FAILURE);
    }

    if (argc == 2) { // Start a new chat group
        string name(argv[1]);
    } else { // Join an existing chat group
        string name(argv[1]);
        string addr_port(argv[2]);
        size_t pos = addr_port.find(":");
        if (pos == string::npos) {
            usage("Error parsing ADDR:PORT");
            exit(EXIT_FAILURE);
        }
        
//        char[] addr = addr_port.substr(0, pos).c_str();
//        char[] port = addr_port.substr(pos + 1, string::npos).c_str();
    }
    std::string ans = create_addr();
    std::cout << ans << std::endl;
}

void usage(string msg) {
    cout << msg << endl;
    cout << "Usage: $ ./dchat USER (start a chat)" << endl;
    cout << "       $ ./dchat USER ADDR:PORT (join a chat)" << endl;
}
