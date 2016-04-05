/**********************************
 * FILE NAME: Member.h
 *
 * DESCRIPTION: Definition of all Member related class
 **********************************/

#ifndef MEMBER_H
#define MEMBER_H

#include "stdincludes.h"

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
    std::string username;
    
    MemberListEntry(std::string address,std::string username);
    MemberListEntry(const MemberListEntry &anotherMLE);
    MemberListEntry& operator =(const MemberListEntry &anotherMLE);

    std::string getAddress() {
        return ip + ":" + std::to_string(port);
    }
    
    std::string getEntry() {
        return getAddress() + ":" + username;
    }
};


/**
 * CLASS NAME: Member
 *
 * DESCRIPTION: Class representing a member in the distributed system
 */
class Member {
    
public:
    Address * address;              // This member's Address
    Address * leaderAddr = nullptr; // The leader's Address
    bool inited = false;            // boolean indicating if this member is up
    bool inGroup;                   // boolean indicating if this member is in the group
    bool bFailed;                   // boolean indicating if this member has failed
    int nnb;                        // number of my neighbors
//    long heartbeat;                 // the node's own heartbeat
//    int pingCounter;                // counter for next ping
//    int timeOutCounter;             // counter for ping timeout
    std::unordered_map<std::string, time_t> heartBeatList; //member will track leader; leader will track every one
    
    std::vector<MemberListEntry> memberList;                // Membership table
    // std::vector<MemberListEntry>::iterator myPos;        // My position in the membership table
    
    std::string getLeaderAddress() {
        return  leaderAddr->getAddress();
    }
    
    // this is the member address list, without user name 
//    std::string getMemberList() {
//        std::string list;
//        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
//            list += (*iter).getAddress();
//            if (iter != memberList.end()-1) list += ":";
//        }
//        return list;
//    }
    
    //with user name
    std::string getMemberList() {
        std::string list;
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            list += (*iter).getEntry();
            if (iter != memberList.end()-1) list += ":";
        }
        return list;
    }

    
    std::string getAddress() {
        return address->getAddress();
    }
    
    void updateHeartBeat(std::string addrKey, time_t heartbeat) {
        heartBeatList[addrKey] = heartbeat;
    }
    
    time_t getHeartBeat(std::string addrKey) {
        //if found, return the heartbeat, otherwise return 0
        std::unordered_map<std::string,int long>::const_iterator got = heartBeatList.find (addrKey);
        if(got == heartBeatList.end()) {
            return NULL;
        } else {
            return got->second;
        }
    }

    Member(std::string addr): inited(false), inGroup(false), bFailed(false), nnb(0){
        this->address = new Address(addr);
    }
    Member(const Member &anotherMember);                // copy constructor
    Member& operator =(const Member &anotherMember);    // Assignment operator overloading
    
    virtual ~Member() {
        delete address;
        delete leaderAddr;
    }
};

#endif /* MEMBER_H */
