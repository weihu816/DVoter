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
    
    int getMsgCnt() {
        return (int)messages.size();
    }
};

class Snapshot {
public:
    DNode *ssnode;
    Member *ssmember;
    int channelNum;
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
    int getChannelNum();
    Member* getMember();
    DNode* getNode();
    std::unordered_map<std::string, Channel> getChannels();
    void setChannels(std::unordered_map<std::string, Channel> c);
};

#endif /* Snapshot_h */
