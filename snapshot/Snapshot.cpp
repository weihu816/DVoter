//
//  Snapshot.cpp
//  dchat
//
//  Created by Yisi XU on 4/17/16.
//  Copyright © 2016 Yisi Xu. All rights reserved.
//

#include "Snapshot.h"
#include "DNode.h"
#include "Member.h"

Snapshot::Snapshot(std::string name, std::string addr) {
    ssnode = new DNode(name, addr);        // Create Member node
    ssmember = new Member(addr);
    ssnode->m_queue = new holdback_queue(0, ssnode);
    status = S_NONE;
    channel_marker_cnt = 0;
    channelNum = 0;
    marker_from_channel = new Channel();
}

void Snapshot::recordState(DNode *cur) {
#ifdef DEBUGLOG
    std::cout << "In SNAPSHOT recording state" << std::endl;
#endif
    Member* nodeMember = cur->getMember();
    ssnode->updateElectionStatus(cur->getElectionStatus());
    // copy message chat queue
    int size = cur->getMessageChatQueueSize();
    for (int i = 0; i < size; i++) {
        ssnode->pushMessageChatQueue(cur->peekMessageChatQueue());
    }
    // copy message send queue
    size = cur->getMessageSendQueueSize();
    for (int i = 0; i < size; i++) {
        ssnode->pushMessageSendQueue(cur->peekMessageSendQueue());
    }
    // copy m_queue
    size = cur->getMQueueSize();
    for (int i = 0; i < size; i++) {
        ssnode->pushMQueue(cur->peekMQueue());
    }
    int x = cur->getMQueueSequenceSeen();
    ssnode->setMQueueSequenceSeen(x);
    ssnode->setMQueueSequenceNext(cur->getMQueueSequenceSeen()+1);

    // copy member information
    ssmember->setInited(nodeMember->getInited());
    ssmember->inGroup = nodeMember->inGroup;
    ssmember->nnb = nodeMember->nnb;
    ssmember-> heartBeatList = nodeMember->heartBeatList;
    ssmember->memberList = nodeMember->memberList;
    ssmember->leaderEntry = nodeMember->leaderEntry;
    auto list = nodeMember->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        if (iter->getAddress().compare(ssmember->getAddress()) != 0) {
            Channel *c = new Channel();
            channelNum++;
    #ifdef DEBUGLOG
            std::cout << "in creating channel loop: " << channelNum << std::endl;
    #endif
            // one channel for each member in member list
            channels[iter->getAddress()] = *c;
            // initialize with false
            channel_markers[iter->getAddress()] = false;
        }
    }
    // create channel for leader
    std::string leader_addr = ssmember->getLeaderAddress();
    if (leader_addr.compare(ssmember->getAddress()) != 0) {
        Channel *c = new Channel();
        channelNum++;
#ifdef DEBUGLOG
        std::cout << "in creating channel for leader: " << channelNum << std::endl;
#endif
        // one channel for each member in member list
        channels[leader_addr] = *c;
        // initialize with false
        channel_markers[leader_addr] = false;
    }
    
}

void Snapshot::recordChannelMarker(std::string from_addr) {
    if (!getChannelMarkerReceived(from_addr)) {
        channel_markers[from_addr] = true;
        channel_marker_cnt++;
    }
}

/**
 * FUNCTION NAME: getChannelMarkerReceived
 *
 * DESCRIPTION: get whether received marker from channel corresponding to 
                addrKey
 */
bool Snapshot::getChannelMarkerReceived(std::string addrKey) {
    auto iter = channel_markers.find(addrKey);
    if (iter == channel_markers.end()) {
        return false;
    } else {
        return iter->second;
    }
}

Channel* Snapshot::getChannel(std::string addrKey) {
    auto iter = channels.find(addrKey);
    if (iter == channels.end()) {
        return nullptr;
    } else {
        return &(iter->second);
    }
}

bool Snapshot::receivedAllMarkers() {
    if (channelNum == channel_marker_cnt) {
        // write to file
        
        // reset status
        
        return true;
    }
    else {
        return false;
    }
}

void Snapshot::setMarkerFromAddr(std::string from_addr) {
    marker_from_addr = from_addr;
}

Channel* Snapshot::getMarkerFromChannel() {
    return marker_from_channel;
}

std::string Snapshot::getMarkerFormAddress() {
    return marker_from_addr;
}
