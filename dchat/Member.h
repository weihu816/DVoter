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
    
    std::string getUsername() {
        return username;
    }
};


/**
 * CLASS NAME: Member
 *
 * DESCRIPTION: Class representing a member in the distributed system
 */
class Member {
    
public:
    Address * address;                                      // This member's Address
    bool inited = false;                                    // boolean indicating if this member is up
    bool inGroup;                                           // boolean indicating if this member is in the group
    int nnb;                                                // number of my neighbors (distributed)
    std::unordered_map<std::string, time_t> heartBeatList;  // Hearbeat List
    
    MemberListEntry * leaderEntry;                          // The leader
    std::vector<MemberListEntry> memberList;                // Membership table
    
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
        std::cout << "NOTICE " << leaderEntry->getUsername() << " left the chat or crashed" << std::endl;
        heartBeatList.erase(leaderEntry->getAddress());
        std::string leader_ip_port = leaderAddr.getAddress();
        leaderEntry = new MemberListEntry(leader_ip_port, leaderName);
        deleteMember(leader_ip_port); //new leader should not be in the list
#ifdef DEBUGLOG
        std::cout << "\tNew Leader: " << getLeaderName() << std::endl;
#endif
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
        time_t curtime;
        updateHeartBeat(ip_port, time(&curtime));
    }

    std::string deleteMember(std::string memberAddr) {
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            if (memberAddr.compare((*iter).getAddress()) == 0 ) {
                std::string memberName = (*iter).username;
                memberList.erase(iter);
                return memberName;
            }
        }
        return "";
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
        // if found, return the heartbeat, otherwise return 0
        auto iter = heartBeatList.find(addrKey);
        if (iter == heartBeatList.end()) {
            return 0;
        } else {
            return iter->second;
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
