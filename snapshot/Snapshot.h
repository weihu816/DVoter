//
//  Snapshot.h
//  dchat
//
//  Created by Yisi XU on 4/17/16.
//  Copyright © 2016 Yisi Xu. All rights reserved.
//

#ifndef Snapshot_hpp
#define Snapshot_hpp

#include "stdincludes.h"

class DNode;
class Member;

class Channel {
public:
    std::queue<std::string> messages;
    
    Channel() {}
    //    // Copy constructor
    //    Channel(const Channel &anotherChannel);
    //    // Overloaded = operator
    //    Channel& operator =(const Channel &anotherChannel);
    
    void addMsg(std::string m) {
        messages.push(m);
    }
    
    std::string retrieveMsg() {
        std::string result = messages.front();
        messages.pop();
        return result;
    }
    
    void clearChannel() {
        while(!messages.empty())
            messages.pop();
    }
    
    bool isEmpty() {
        return messages.empty();
    }
    
    void copyChannelMsg(Channel *c) {
        while (!c->isEmpty()) {
            std::string msg = c->retrieveMsg();
            messages.push(msg);
        }
    }
};

class Snapshot {
public:
//    int seq;
    DNode *ssnode = nullptr;
    Member *ssmember;
    int channelNum;
    int status;
    int channel_marker_cnt;
    std::string marker_from_addr;
    Channel *marker_from_channel;
    
    std::unordered_map<std::string, bool> channel_markers;  // <address, received_marker> pair
    
    std::unordered_map<std::string, Channel> channels;  // <address, channel> pair
    
    Snapshot(std::string name, std::string addr);
    void recordState(DNode* cur);
    void recordChannelMarker(std::string from_addr);
    bool getChannelMarkerReceived(std::string addrKey);
    bool receivedAllMarkers();
    Channel* getChannel(std::string addrKey);
    void setMarkerFromAddr(std::string from_addr);
    Channel* getMarkerFromChannel();
    std::string getMarkerFormAddress();
    
};

#endif /* Snapshot_h */
