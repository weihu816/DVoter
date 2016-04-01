/**********************************
 * FILE NAME: Member.h
 *
 * DESCRIPTION: Definition of all Member related class
 **********************************/

#ifndef MEMBER_H
#define MEMBER_H

#include "stdincludes.h"


/**
 * CLASS NAME: q_elt
 *
 * DESCRIPTION: Entry in the queue
 */
class q_elt {
public:
    void *elt;
    int size;
    q_elt(void *elt, int size);
};


/**
 * CLASS NAME: Address
 *
 * DESCRIPTION: Class representing the address of a single node
 */
class Address {
public:
    std::string ip;
    int port;
    Address() {}
    // Copy constructor
    Address(const Address &anotherAddress);
    // Overloaded = operator
    Address& operator =(const Address &anotherAddress);
    bool operator ==(const Address &anotherAddress);
    Address(std::string address) {
        size_t pos = address.find(":");
        ip = address.substr(0, pos);
        port = stoi(address.substr(pos + 1, address.size()-pos-1));
    }
    std::string getAddress() {
        return ip + ":" + std::to_string(port);
    }
    void init() {
        ip.clear();
    }
};


/**
 * CLASS NAME: MemberListEntry
 *
 * DESCRIPTION: Entry in the membership list
 */
class MemberListEntry {
public:
    std::string ip;
    int port;
    long heartbeat = 0;
    long timestamp = 0;
    
    MemberListEntry(std::string address, long heartbeat, long timestamp);
    MemberListEntry(std::string address); 
    MemberListEntry(): ip(0), port(0), heartbeat(0), timestamp(0) {}
    MemberListEntry(const MemberListEntry &anotherMLE);
    MemberListEntry& operator =(const MemberListEntry &anotherMLE);

    std::string getAddress() {
        return ip + ":" + std::to_string(port);
    }
};


/**
 * CLASS NAME: Member
 *
 * DESCRIPTION: Class representing a member in the distributed system
 */
class Member {
    
public:
    Address * address;      // This member's Address
    Address * leaderAddr = nullptr; // The leader's Address
    bool inited = false;    // boolean indicating if this member is up
    bool inGroup;           // boolean indicating if this member is in the group
    bool bFailed;           // boolean indicating if this member has failed
    int nnb;                // number of my neighbors
    long heartbeat;         // the node's own heartbeat
    int pingCounter;        // counter for next ping
    int timeOutCounter;     // counter for ping timeout
    bool isLeader = false;
    
    std::vector<MemberListEntry> memberList;            // Membership table
    // std::vector<MemberListEntry>::iterator myPos;       // My position in the membership table
    
    std::string getLeaderAddress() {
        return  leaderAddr->getAddress();
    }
    
    std::string getMemberList() {
        std::string list;
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            list += (*iter).getAddress();
            if (iter != memberList.end()-1) list += ":";
        }
        return list;
    }
    
    std::string getMemberList() {
        std::string list;
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            list += (*iter).getAddress();
            if (iter != memberList.end()-1) list += ":";
        }
        return list;
    }
    
    std::string getAddress() {
        return address->getAddress();
    }
    /**
     * Constructor
     */
    Member(std::string addr): inited(false), inGroup(false), bFailed(false), nnb(0), heartbeat(0), pingCounter(0), timeOutCounter(0) {
        this->address = new Address(addr);
    }
    // copy constructor
    Member(const Member &anotherMember);
    // Assignment operator overloading
    Member& operator =(const Member &anotherMember);
    
    virtual ~Member() {
        delete address;
    }
};

#endif /* MEMBER_H */
