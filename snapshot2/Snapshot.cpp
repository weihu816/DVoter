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

/***
 * Constructor of Snapshot
 ***/
Snapshot::Snapshot(std::string name, std::string addr) {
    ssnode = new DNode(name, addr);        // Create Member node
    ssmember = new Member(addr);
    ssnode->m_queue = new holdback_queue(0, ssnode);
    channel_marker_cnt = 0;
    channelNum = 0;
    marker_from_channel = new Channel();
}

/***
 * FUNCTION NAME: recordState
 *
 * DESCRIPTION: record current state of process
 ***/
void Snapshot::recordState(DNode *cur) {
#ifdef DEBUGLOG
    std::cout << "In SNAPSHOT recording state" << std::endl;
#endif
    Member* nodeMember = cur->getMember();
    ssnode->updateElectionStatus(cur->getElectionStatus());

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
        // one channel for each member in member list
        channels[leader_addr] = *c;
        // initialize with false
        channel_markers[leader_addr] = false;
    }
    
    // copy message queue
    ssnode->setDisplayMessageQueue(cur->getDisplayMessageQueue());
}

/***
 * FUNCTION NAME: recordChannelMarker
 *
 * DESCRIPTION: record where a marker comes from
 ***/
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

/***
 * FUNCTION NAME: getChannel
 *
 * DESCRIPTION: get channel by address
 ***/
Channel* Snapshot::getChannel(std::string addrKey) {
    auto iter = channels.find(addrKey);
    if (iter == channels.end()) {
        return nullptr;
    } else {
        return &(iter->second);
    }
}

/***
 * FUNCTION NAME: receivedAllMarkers
 *
 * DESCRIPTION: check whether all the markers from other processes 
 *              have been received
 ***/
bool Snapshot::receivedAllMarkers() {
    if (channelNum == channel_marker_cnt) {
        return true;
    }
    else {
        return false;
    }
}

/***
 * FUNCTION NAME: setMarkerFromAddr
 *
 * DESCRIPTION: set marker_from_addr, indicating through which process
 *              the marker is received
 ***/
void Snapshot::setMarkerFromAddr(std::string from_addr) {
    marker_from_addr = from_addr;
}

/////////////////////////// GETTERS ///////////////////////////////
/**
 * get channel who initialzed the snapshot
 */
Channel* Snapshot::getMarkerFromChannel() {
    return marker_from_channel;
}

/**
 * snapshot initializer address getter
 */
std::string Snapshot::getMarkerFormAddress() {
    return marker_from_addr;
}

/**
 * channel number getter
 */
int Snapshot::getChannelNum() {
    return channelNum;
}

/**
 * get member to whom this snapshot belongs to
 */
Member* Snapshot::getMember() {
    return ssmember;
}

/**
 * get node to which this snapshot belongs to
 */
DNode* Snapshot::getNode() {
    return ssnode;
}

/***
 * channels map getter
 ***/
std::unordered_map<std::string, Channel> Snapshot::getChannels() {
    return channels;
}

/***
 * channels setter
 ***/
void Snapshot::setChannels(std::unordered_map<std::string, Channel> c) {
    channels = c;
}
