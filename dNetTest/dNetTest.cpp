//
//  main.cpp
//  dNetTest
//
//  Created by weihu on 4/2/16.
//  Copyright © 2016 DVoter. All rights reserved.
//

#include <iostream>
#include "DNet.h"

using namespace std;

int main(int argc, const char * argv[]) {
    DNet * dNet = new DNet;
    cout << "Start listening on: " << dNet->DNinit() << endl;
    std::string data;
    Address addr;
    while (dNet->DNrecv(addr, data)) {
        cout << "Recv: " << data << endl;
    }
    delete dNet;
    return 0;
}
