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
    std::string addr;
    int port;
    Address() {}
    Address(const Address &anotherAddress);             // Copy constructor
    Address& operator =(const Address &anotherAddress); // Overloaded = operator
    bool operator ==(const Address &anotherAddress);    // Overloaded == operator
    Address(std::string address) {
        size_t pos = address.find(":");
        addr = address.substr(0, pos);
        port = stoi(address.substr(pos + 1, address.size()-pos-1));
    }
    std::string getAddress() {
        return addr + ":" + std::to_string(port);
    }
    void init() {
        addr.clear();
    }
};

/**
 * CLASS NAME: MemberListEntry
 *
 * DESCRIPTION: Entry in the membership list
 */
class MemberListEntry {
public:
    int id;
    short port;
    long heartbeat;
    long timestamp;
    MemberListEntry(int id, short port, long heartbeat, long timestamp);
    MemberListEntry(int id, short port);
    MemberListEntry(): id(0), port(0), heartbeat(0), timestamp(0) {}
    MemberListEntry(const MemberListEntry &anotherMLE);
    MemberListEntry& operator =(const MemberListEntry &anotherMLE);
    int getid();
    short getport();
    long getheartbeat();
    long gettimestamp();
    void setid(int id);
    void setport(short port);
    void setheartbeat(long hearbeat);
    void settimestamp(long timestamp);
};

/**
 * CLASS NAME: Member
 *
 * DESCRIPTION: Class representing a member in the distributed system
 */
class Member {
    
public:
    Address addr;           // This member's Address
    bool inited = false;    // boolean indicating if this member is up
    bool inGroup;           // boolean indicating if this member is in the group
    bool bFailed;           // boolean indicating if this member has failed
    int nnb;                // number of my neighbors
    long heartbeat;         // the node's own heartbeat
    int pingCounter;        // counter for next ping
    int timeOutCounter;     // counter for ping timeout
    
    
    std::vector<MemberListEntry> memberList;            // Membership table
    std::vector<MemberListEntry>::iterator myPos;       // My position in the membership table
    std::queue<q_elt> mp1q;                             // Queue for failure detection messages
    
    /**
     * Constructor
     */
    Member(): inited(false), inGroup(false), bFailed(false), nnb(0), heartbeat(0), pingCounter(0), timeOutCounter(0) {}
    
    Member(const Member &anotherMember);                // copy constructor
    Member& operator =(const Member &anotherMember);    // Assignment operator overloading
    
    /**
     *  Destructor
     */
    virtual ~Member() {}
};

#endif /* MEMBER_H */
