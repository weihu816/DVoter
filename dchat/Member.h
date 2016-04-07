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
    std::string getAddressIp() {
        return ip;
    }
    std::string getAddressPort() {
        return std::to_string(port);
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
    
    MemberListEntry(std::string address, std::string name);
    MemberListEntry(std::string entry);
    MemberListEntry(const MemberListEntry &anotherMLE);
    MemberListEntry& operator =(const MemberListEntry &anotherMLE);
    bool operator ==(const MemberListEntry &anotherMLE);

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
    Address * address;                      // This member's Address
    bool inited = false;                    // boolean indicating if this member is up
    bool inGroup;                           // boolean indicating if this member is in the group
    int nnb;                                // number of my neighbors (distributed)
    std::unordered_map<std::string, time_t> heartBeatList;
    std::vector<MemberListEntry> memberList;                // Membership table

    MemberListEntry * leaderEntry;                          // The leader

    std::string getLeaderAddress() {
        return  leaderEntry->getAddress();
    }
    std::string getLeaderAddressIp() {
        return leaderEntry->ip;
    }
    std::string getLeaderAddressPort() {
        return std::to_string(leaderEntry->port);
    }
    std::string getLeaderName() {
        return  leaderEntry->username;
    }
    
    bool isLeader() {
        return (getLeaderAddress().compare(address->getAddress()) == 0);
    }
    
    void updateLeader(Address leaderAddr, std::string leaderName) {
        std::string ip = leaderAddr.getAddressIp();
        std::string port = leaderAddr.getAddressPort();
        leaderEntry = new MemberListEntry(ip + ":" + port, leaderName);
    }

    
    // this is the member address list, without user name
    void addMember(std::string ip_port, std::string name) {
        MemberListEntry entry(ip_port, name);
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            if ((*iter).getEntry() ==  entry.getEntry()) return;
        }
#ifdef DEBUGLOG
        std::cout << "\tMember::addMember: " << entry.getEntry() << std::endl;
#endif
        memberList.push_back(entry);
    }

    void deleteMember(MemberListEntry entry) {
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            if ((*iter).getEntry() ==  entry.getEntry()) {
                memberList.erase(iter);
            }
        }
    }

    std::string getMemberList() {
        std::string list = "";
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            list += (*iter).getEntry();
            if (iter != memberList.end()-1) list += "#";
        }
        return list;
    }
    
    void printMemberList() {
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            std::cout << (*iter).username << " " << (*iter).getAddress() << std::endl;
        }
    }
    
    std::vector<MemberListEntry> getMemberEntryList() {
        return memberList;
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
            return 0;
        } else {
            return got->second;
        }
    }

    Member(std::string addr): inited(false), inGroup(false), nnb(0){
        this->address = new Address(addr);
    }
    Member(const Member &anotherMember);                // copy constructor
    Member& operator =(const Member &anotherMember);    // Assignment operator overloading
    
    virtual ~Member() {
        delete address;
        delete leaderEntry;
    }
};

#endif /* MEMBER_H */
