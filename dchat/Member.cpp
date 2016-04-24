/**********************************
 * FILE NAME: Member.cpp
 *
 * DESCRIPTION:
 **********************************/

#include "Member.h"

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
    this->memberList = anotherMember.memberList;
}

/**
 * Assignment operator overloading
 */
Member& Member::operator =(const Member& anotherMember) {
    *this->address = *anotherMember.address;
    *this->leaderEntry = *anotherMember.leaderEntry;
    this->inited = anotherMember.inited;
    this->inGroup = anotherMember.inGroup;
    this->memberList = anotherMember.memberList;
    return *this;
}

/**
 * FUNCTION NAME: updateLeader
 *
 * DESCRIPTION: Update Leader information
 */
void Member::updateLeader(Address leaderAddr, std::string leaderName) {
    
    if (getLeaderAddress().compare(leaderAddr.getAddress()) == 0) return;
    std::cout << "NOTICE " << leaderEntry->getUsername() << " left the chat or crashed." << std::endl;
    std::string leader_ip_port = leaderAddr.getAddress();
    leaderEntry = new MemberListEntry(leader_ip_port, leaderName);
    
    std::cout << "NOTICE " << getLeaderName() << " is the new leader" << std::endl;
    // New leader should not be in the list
    deleteMember(leader_ip_port);
    
#ifdef DEBUGLOG
    std::cout << "\tNew Leader: " << getLeaderName() << std::endl;
#endif
    
}

/**
 * FUNCTION NAME: addMember
 *
 * DESCRIPTION: this is the member address list, without user name
 */
void Member::addMember(std::string ip_port, std::string name) {
    
    std::unique_lock<std::mutex> lk(mutex_memberList);
    
    MemberListEntry entry(ip_port, name);
    for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
        if ((*iter).getEntry() ==  entry.getEntry()) return;
    }
#ifdef DEBUGLOG
    std::cout << "\tMember::addMember: " << entry.getEntry() << std::endl;
#endif
    memberList.push_back(entry);
    
}

/**
 * FUNCTION NAME: deleteMember
 *
 * DESCRIPTION: Delete the member from the memberlist given member address
 */
std::string Member::deleteMember(std::string memberAddr) {
    
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

/**
 * FUNCTION NAME: getMemberList
 *
 * DESCRIPTION: get MemberList
 */
std::string Member::getMemberList() {
    
    std::unique_lock<std::mutex> lk(mutex_memberList);
    
    std::string list = "";
    for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
        list += (*iter).getEntry();
        if (iter != memberList.end()-1) list += "#";
    }
    return list;
    
}


/**
 * FUNCTION NAME: getMemberListGUI
 *
 * DESCRIPTION: getMemberListGUI
 */
std::string Member::getMemberListGUI() {
    
    std::unique_lock<std::mutex> lk(mutex_memberList);
    std::string str = getLeaderAddress() + " " + getLeaderName() + " (Leader)" +"\n";
    for (auto iter = memberList.begin(); iter != memberList.end(); iter++) {
        str += (*iter).getAddress() + " " + (*iter).getUsername();
        if (iter != memberList.end()-1) str += "\n";
    }
    return str;
    
}