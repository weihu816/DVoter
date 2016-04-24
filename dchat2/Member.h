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
private:
    std::mutex mutex_memberList;
    std::vector<MemberListEntry> memberList;                // Membership table

public:
    Address * address;                                      // This member's Address
    bool inited = false;                                    // boolean indicating if this member is up
    bool inGroup;                                           // boolean indicating if this member is in the group
    
    Member(std::string addr): inited(false), inGroup(false) { this->address = new Address(addr); }
    Member(const Member &anotherMember);
    Member& operator =(const Member &anotherMember);
    virtual ~Member() { delete address; }
    
    std::vector<MemberListEntry> getMemberEntryList() { return memberList; }
    std::string getAddress() { return address->getAddress(); }
    
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
     * FUNCTION NAME: getMemberList
     *
     * DESCRIPTION: get MemberList
     */
    std::string getMemberListStr() {
        std::unique_lock<std::mutex> lk(mutex_memberList);
        std::string list = "";
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            list += (*iter).getEntry();
            if (iter != memberList.end() - 1) list += "#";
        }
        return list;
    }
    
    /**
     * FUNCTION NAME: addMember
     *
     * DESCRIPTION: this is the member address list, without user name
     */
    void addMember(std::string ip_port, std::string name) {
        std::unique_lock<std::mutex> lk(mutex_memberList);
        MemberListEntry entry(ip_port, name);
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            if ((*iter).getEntry() ==  entry.getEntry()) return;
        }
        memberList.push_back(entry);
    }
    
    /**
     * FUNCTION NAME: existsMember
     *
     * DESCRIPTION: if the member exists
     */
    bool existsMember(std::string ip_port) {
        std::unique_lock<std::mutex> lk(mutex_memberList);
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            if ((*iter).getAddress() == ip_port) return true;
        }
        return false;
    }
    
    /**
     * FUNCTION NAME: deleteMember
     *
     * DESCRIPTION: Delete the member from the memberlist given member address
     */
    std::string deleteMember(std::string memberAddr) {
        std::unique_lock<std::mutex> lk(mutex_memberList);
        for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
            if (memberAddr.compare((*iter).getAddress()) == 0 ) {
                std::string memberName = (*iter).username;
                memberList.erase(iter);
                return memberName;
            }
        }
        return "";
    }
};

#endif /* MEMBER_H */
