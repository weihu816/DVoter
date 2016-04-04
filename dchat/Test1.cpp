#include <iostream>
#include "DNet.h"
#include "BlockingQueue.h"

using namespace std;

int main(int argc, const char * argv[]) {
    DNet * dNet = new DNet;
    dNet->DNinit();
    std::string addr;
    dNet->DNinfo(addr);
    cout << addr << endl;
    Address from_addr;
    std::string data;
    while (dNet->DNrecv(from_addr, data) != FAILURE) {
        cout << "Receive: " << data << " from " << from_addr.getAddress() << endl;
    }
    delete dNet;
}