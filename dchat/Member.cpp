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
 * Constuctor: holds the member address; 
 * Main use: memberList addr track
 */
MemberListEntry::MemberListEntry(std::string address, std::string name) : username(name) {
    size_t pos = address.find_last_of(":");
    ip = address.substr(0, pos);
    port = stoi(address.substr(pos + 1));
}

MemberListEntry::MemberListEntry(std::string entry) {
    size_t pos_1 = entry.find_last_of(":");
    username = entry.substr(pos_1 + 1);
    std::string ip_port = entry.substr(0, pos_1);
    size_t pos_2 = ip_port.find_last_of(":");
    ip = ip_port.substr(0, pos_2);
    port = stoi(ip_port.substr(pos_2 + 1));
}


/**
 * Copy constructor
 */
MemberListEntry::MemberListEntry(const MemberListEntry &anotherMLE) {
    this->ip = anotherMLE.ip;
    this->port = anotherMLE.port;
    this->username = anotherMLE.username;
}

/**
 * Assignment operator overloading
 */
MemberListEntry& MemberListEntry::operator =(const MemberListEntry &anotherMLE) {
    this->ip = anotherMLE.ip;
    this->port = anotherMLE.port;
    this->username = anotherMLE.username;
    return *this;
}

/**
 * Compare two MemberListEntry objects
 */
bool MemberListEntry::operator ==(const MemberListEntry& anotherEntry) {
    return (ip.compare(anotherEntry.ip) == 0) && (port == anotherEntry.port) && (username == anotherEntry.username);
}

/**
 * Copy Constructor
 */
Member::Member(const Member &anotherMember) {
    *this->address = *anotherMember.address;
    *this->leaderEntry = *anotherMember.leaderEntry;
    this->inited = anotherMember.inited;
    this->inGroup = anotherMember.inGroup;
    this->nnb = anotherMember.nnb;
//    this->heartbeat = anotherMember.heartbeat;
//    this->pingCounter = anotherMember.pingCounter;
//    this->timeOutCounter = anotherMember.timeOutCounter;
    this->memberList = anotherMember.memberList;
    // this->myPos = anotherMember.myPos;
}

/**
 * Assignment operator overloading
 */
Member& Member::operator =(const Member& anotherMember) {
    *this->address = *anotherMember.address;
    *this->leaderEntry = *anotherMember.leaderEntry;
    this->inited = anotherMember.inited;
    this->inGroup = anotherMember.inGroup;
    this->nnb = anotherMember.nnb;
//    this->heartbeat = anotherMember.heartbeat;
//    this->pingCounter = anotherMember.pingCounter;
//    this->timeOutCounter = anotherMember.timeOutCounter;
    this->memberList = anotherMember.memberList;
    // this->myPos = anotherMember.myPos;
    return *this;
}