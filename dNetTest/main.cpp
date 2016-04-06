//
//  main.cpp
//  dNetTest
//
//  Created by weihu on 4/2/16.
//  Copyright © 2016 DVoter. All rights reserved.
//

#include <iostream>
#include "DNet.h"
#include "BlockingQueue.h"

using namespace std;

void testQueuePop(blocking_queue<string> * q) {
    cout << q->pop() << endl;
    cout << q->pop() << endl;
    q->push("!!!");
    cout << q->pop() << endl;
}

void testQueuePush(blocking_queue<string> * q) {
    q->push("Hello");
    q->push("World");
}

void testQueue() {
    blocking_queue<string> q;
    
    thread thread_pop(testQueuePop, &q);
    cout << "sleep for 3000 ms" << endl;
    std::this_thread::sleep_for (std::chrono::seconds(3));
    thread thread_push(testQueuePush, &q);
    
    thread_pop.join();
    thread_push.join();
}

void testHoldBackQueue() {
    holdback_queue pq(1);
    pq.push(std::make_pair(3, "aa"));
    pq.push(std::make_pair(4, "bb"));
    pq.push(std::make_pair(2, "cc"));
    cout << pq.pop().first << endl;
    cout << pq.pop().first << endl;
    cout << pq.pop().first << endl;
}

int main(int argc, const char * argv[]) {
    string recv_msg = "HELLO#WORLD#KKK";
    char * cstr = new char[recv_msg.length() + 1];
    strcpy(cstr, recv_msg.c_str());
    cout << cstr << endl;
    char * msg_type = strtok(cstr, "#");
    cout << msg_type << endl;
    cout << cstr << endl;
    cout << strtok(NULL, "#") << endl;
    cout << cstr << endl;
    cout << strtok(NULL, "#") << endl;
}






