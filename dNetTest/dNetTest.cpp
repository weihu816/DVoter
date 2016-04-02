//
//  main.cpp
//  dNetTest
//
//  Created by weihu on 4/2/16.
//  Copyright © 2016 DVoter. All rights reserved.
//

#include <iostream>
#include "DNet.h"
#include "BlockingPriorityQueue.h"
#include "BlockingQueue.h"

using namespace std;

void testQueue() {
    
    blocking_queue<string> q;
    q.push("Hello");
    q.push("World");
    cout << q.pop() << endl;
    cout << q.pop() << endl;
    
    blocking_priority_queue pq;
    pq.push(std::pair<int, string>(2, "aa"));
    pq.push(std::pair<int, string>(1, "bb"));
    pq.push(std::pair<int, string>(3, "cc"));
    cout << pq.pop().first << endl;
    cout << pq.pop().first << endl;
    cout << pq.pop().first << endl;
    
}

void testDNet() {
    DNet * dNet = new DNet;
    cout << "Start listening on: " << dNet->DNinit() << endl;
    std::string data;
    Address addr;
    while (dNet->DNrecv(addr, data)) {
        cout << "Recv: " << data << endl;
    }
    delete dNet;
}
int main(int argc, const char * argv[]) {
    
    testQueue();

}


