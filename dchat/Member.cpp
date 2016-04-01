/**********************************
 * FILE NAME: Member.cpp
 *
 * DESCRIPTION:
 **********************************/

#include "Member.h"

///**
// * Constructor
// */
//q_elt::q_elt(void *elt, int size): elt(elt), size(size) {}

/**
 * Copy constructor
 */
Address::Address(const Address &anotherAddress) {
    ip = anotherAddress.ip;
    port = anotherAddress.port;
}

/**
 * Assignment operator overloading
 */
Address& Address::operator =(const Address& anotherAddress) {
    ip = anotherAddress.ip;
    port = anotherAddress.port;
    return *this;
}

/**
 * Compare two Address objects
 * Return true/non-zero if they have the same ip address and port number
 * Return false/zero if they are different
 */
bool Address::operator ==(const Address& anotherAddress) {
    return (ip.compare(anotherAddress.ip) == 0) && (port == anotherAddress.port);
}

/**
 * Constructor
 */
MemberListEntry::MemberListEntry(std::string address, long heartbeat, long timestamp): heartbeat(heartbeat), timestamp(timestamp) {
    size_t pos = address.find(":");
    ip = address.substr(0, pos);
    port = stoi(address.substr(pos + 1, address.size()-pos-1));
}

/**
 * Constuctor
 */
MemberListEntry::MemberListEntry(std::string address, bool isLeader): isLeader(isLeader) {
    size_t pos = address.find(":");
    ip = address.substr(0, pos);
    port = stoi(address.substr(pos + 1, address.size()-pos-1));
}


/**
 * Copy constructor
 */
MemberListEntry::MemberListEntry(const MemberListEntry &anotherMLE) {
    this->heartbeat = anotherMLE.heartbeat;
    this->ip = anotherMLE.ip;
    this->port = anotherMLE.port;
    this->timestamp = anotherMLE.timestamp;
}

/**
 * Assignment operator overloading
 */
MemberListEntry& MemberListEntry::operator =(const MemberListEntry &anotherMLE) {
    MemberListEntry temp(anotherMLE);
    std::swap(heartbeat, temp.heartbeat);
    std::swap(ip, temp.ip);
    std::swap(port, temp.port);
    std::swap(timestamp, temp.timestamp);
    return *this;
}

/**
 * Copy Constructor
 */
Member::Member(const Member &anotherMember) {
    *this->address = *anotherMember.address;
    this->inited = anotherMember.inited;
    this->inGroup = anotherMember.inGroup;
    this->bFailed = anotherMember.bFailed;
    this->nnb = anotherMember.nnb;
    this->heartbeat = anotherMember.heartbeat;
    this->pingCounter = anotherMember.pingCounter;
    this->timeOutCounter = anotherMember.timeOutCounter;
    this->memberList = anotherMember.memberList;
    // this->myPos = anotherMember.myPos;
}

/**
 * Assignment operator overloading
 */
Member& Member::operator =(const Member& anotherMember) {
    *this->address = *anotherMember.address;
    this->inited = anotherMember.inited;
    this->inGroup = anotherMember.inGroup;
    this->bFailed = anotherMember.bFailed;
    this->nnb = anotherMember.nnb;
    this->heartbeat = anotherMember.heartbeat;
    this->pingCounter = anotherMember.pingCounter;
    this->timeOutCounter = anotherMember.timeOutCounter;
    this->memberList = anotherMember.memberList;
    // this->myPos = anotherMember.myPos;
    return *this;
}