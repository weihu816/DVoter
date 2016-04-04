#include <iostream>
#include "DNet.h"
#include "BlockingQueue.h"

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc != 2) return 1;
    DNet * dNet = new DNet;
    dNet->DNinit();
    std::string addr;
    dNet->DNinfo(addr);
    cout << addr << endl;
    string addr_remote(argv[1]);
    Address send_addr(addr_remote);
    std::string data("huweiwoaini"), ack;
    dNet->DNsend(&send_addr, data, ack, 1);
    cout << ack << endl;
    delete dNet;
}