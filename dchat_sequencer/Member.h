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
 * DESCRIPTION: Address class
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
private:
    std::mutex mutex_memberList;

public:
    Address * address;                                      // This member's Address
    bool inited = false;                                    // boolean indicating if this member is up
    bool inGroup;                                           // boolean indicating if this member is in the group
    std::vector<MemberListEntry> memberList;                // Membership table
    MemberListEntry * leaderEntry;                          // The leader
    
    Member(std::string addr): inited(false), inGroup(false) {
        this->address = new Address(addr);
    }
    Member(const Member &anotherMember);                    // copy constructor
    Member& operator =(const Member &anotherMember);        // Assignment operator overloading
    
    virtual ~Member() {
        delete address;
        delete leaderEntry;
    }
    
    
    /*---------- Getter Methods ----------*/
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
    
    void updateLeader(Address leaderAddr, std::string leaderName);
    void addMember(std::string ip_port, std::string name);
    std::string deleteMember(std::string memberAddr);
    std::string getMemberList();
    std::string getMemberListGUI();
    
    
    /**
     * FUNCTION NAME: printMemberList
     *
     * DESCRIPTION: print MemberList
     */
    void printMemberList() {
        
        std::unique_lock<std::mutex> lk(mutex_memberList);
        
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            std::cout << (*iter).username << " " << (*iter).getAddress() << std::endl;
        }
    }
    
    
    /**
     * FUNCTION NAME: getMemberEntryList
     *
     * DESCRIPTION: getMemberEntryList
     */
    std::vector<MemberListEntry> getMemberEntryList() {
        return memberList;
    }
    
    
    /**
     * FUNCTION NAME: getAddress
     *
     * DESCRIPTION: getAddress
     */
    std::string getAddress() {
        return address->getAddress();
    }
    
};

#endif /* MEMBER_H */
