/**********************************
 * FILE NAME: DNode.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of DNode class functions.
 **********************************/

#include "DNode.h"
#include "Handler.h"

/**
 * CONSTRUCTOR
 */
DNode::DNode(std::string name, std::string join_addr) : username(name) {
    dNet = new DNet(new Handler(this));
    dNet->DNinit();
    std::string my_addr;
    dNet->DNinfo(my_addr);
    member_node = new Member(my_addr);        // Create Member node
    if (!join_addr.empty()) {
        join_address = join_addr;
    } else {
        join_address = my_addr;
    }
    snapshotCnt = 0;
}

//////////////////////////////// THREAD FUNC ////////////////////////////////

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int DNode::recvLoop() {
    Address addr;
    std::string data;
    dNet->DNrecv(addr, data);
    return SUCCESS;
}

/**
 * FUNCTION NAME: msgLoop
 *
 * DESCRIPTION: This function receives chat messages and displays to terminal
 */
std::string DNode::msgLoop() {
//#ifdef DEBUGLOG
//    std::cout << "\tpop from message_chat_queue" << std::endl;
//#endif
    return message_chat_queue.pop();
}

//////////////////////////////// NODE INIT AND CLEAN FUNC ////////////////////////////////


/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
int DNode::nodeStart() {
    // Self booting routines
    if (member_node->getAddress().compare(join_address) != 0) {
        std::cout << username << " joining a new chat on " << join_address;
        std::cout << ", listening on " << member_node->getAddress() << std::endl;
        // for snapshot
        std::string msg(username + " joining a new chat on " + join_address +
                        ", listening on " + member_node->getAddress());
        addDisplayMsg(msg);
    }
    if( initThisNode() == FAILURE ) {
        std::cout << "init_thisnode failed. Exit." << std::endl;
        return FAILURE;
    }
    if( introduceSelfToGroup(join_address, false) == FAILURE ) {
        std::cout << "Sorry no chat is active on " << member_node->getAddress()
        << ", try again later.\nBye." << std::endl;
        return FAILURE;
    }
    std::cout << "Succeed, current users:" << std::endl; // TODO
    std::cout << member_node->getLeaderName() << " " << member_node->getLeaderAddress() << " (Leader)" << std::endl;
    member_node->printMemberList();
    if (member_node->isLeader()) std::cout << "Waiting for others to join" << std::endl;
    // for snapshot
    std::string msg("Start chatting. Current users: \n" + member_node->getLeaderName() + " " +
                    member_node->getLeaderAddress() + " (Leader)\n");
    std::vector<MemberListEntry> list(member_node->getMemberEntryList());
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        msg += (*iter).username + " " + (*iter).getAddress() + "\n";
    }
    addDisplayMsg(msg);

    return SUCCESS;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int DNode::initThisNode() {
    member_node->inited = true;
    member_node->inGroup = false;
    member_node->nnb = 0;
    member_node->memberList.clear();
    return SUCCESS;
}


/**
 * FUNCTION NAME: nodeLeave
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int DNode::nodeLeave() {    
    return SUCCESS;
}



//////////////////////////////// MEMBER LIST UPDATE FUNC ////////////////////////////////

/**
 * FUNCTION NAME: initMemberList
 *
 * DESCRIPTION: initialize member list given member_list like the following
 *              ip1:port1:name1:ip2:port2:name2: ...
 */
void DNode::initMemberList(std::string member_list, std::string leaderAddr) {
    
#ifdef DEBUGLOG
    std::cout << "\tDNode::initMemberList: " << member_list  << " leaderAddr:" << leaderAddr << std::endl;
#endif

    if (member_list.empty()) return;
    char * cstr = new char[member_list.length() + 1];
    std::string addr;
    strcpy(cstr, member_list.c_str());
    char * pch = strtok(cstr, "#");
    while (pch != NULL) {
        std::string entry(pch);
        MemberListEntry listEntry(entry);
        addMember(listEntry.getAddress(), listEntry.username, false);
        pch = strtok(NULL, "#");
    }
    
}

/**
 * FUNCTION NAME: addMember
 *
 * DESCRIPTION:
 */
void DNode::addMember(std::string ip_port, std::string name, bool toPrint){
    member_node->addMember(ip_port, name);
    if (toPrint) {
        std::cout << "NOTICE " << name << " joined on " << ip_port << std::endl;
        // for snapshot
        std::string msg("NOTICE " + name + " joined on " + ip_port);
        addDisplayMsg(msg);
    }
}


/**
 * FUNCTION NAME: deleteMember
 *
 * DESCRIPTION: delete member from the memberList
 */
void DNode::deleteMember(std::string memberAddr){
    std::string memberName = member_node->deleteMember(memberAddr);
    // deal with case where in taking snapshot but member left
    if (getSnapshotStatus() == S_RECORDING) {
        // delete the channel
        snapshot->channels.erase(memberAddr);
        snapshot->channelNum--;
    }
    if(!memberName.empty()) {
        std::cout << "NOTICE " << memberName << " left the chat or crashed" << std::endl;
        // for snapshot
        std::string msg("NOTICE " + memberName + " left the chat or crashed");
        addDisplayMsg(msg);
    }
}



//////////////////////////////// CLIENT: OPERATIONS FUNC ////////////////////////////////

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int DNode::introduceSelfToGroup(std::string join_addr, bool isSureLeaderAddr) {

    std::string self_addr = member_node->getAddress();

    if (self_addr.compare(join_addr) == 0) {
        
        // I am the group booter (first process to join the group). Boot up the group
        std::cout << username << " started a new chat, listening on " << member_node->getAddress() << std::endl;
        // addMember(join_addr, username, true); // I'm the leader
        member_node->leaderEntry = new MemberListEntry(join_addr, username);
        
        m_queue = new holdback_queue(0, this);
        
    } else {

        Address to_addr(join_addr);
        // Requst to join by contacting the member
        std::string msg_to = std::string(D_JOINREQ) + "#" + std::to_string(member_node->address->port) + "#" + username;
        std::string msg_ack;
        if (dNet->DNsend(&to_addr, msg_to, msg_ack, 3) != SUCCESS) return FAILURE;
        // Receive member lists from the leader, then save the leader address

        // send JOINREQ message to introducer member
        size_t index = msg_ack.find("#");
        if (index == std::string::npos) return FAILURE;
        std::string msg_type = msg_ack.substr(0, index);
        
        if (msg_type.compare(D_JOINLEADER) == 0) {
            
            // received: JOINLEADER#LEADERIP#LEADERPORT#LEADERNAME
            if (isSureLeaderAddr) return FAILURE;
            std::string ip_port_name = msg_ack.substr(index + 1);
            std::string ip = ip_port_name.substr(0, ip_port_name.find("#"));
            std::string port_name = ip_port_name.substr(ip_port_name.find("#") + 1);
            std::string port = port_name.substr(0, port_name.find("#"));
            std::string name = port_name.substr(port_name.find("#") + 1);
            member_node->leaderEntry = new MemberListEntry(ip + ":" + port, name);
            return this->introduceSelfToGroup(member_node->getLeaderAddress(), true);
            
        } else if (msg_type.compare(D_JOINLIST) == 0) {
            
            // received: JOINLIST#initSeq#leadername#ip1:port1:name1:ip2:port2:name2...
            std::string recv_param = msg_ack.substr(index + 1);
            std::string recv_init_seq = recv_param.substr(0, recv_param.find("#"));
            std::string recv_name_list = recv_param.substr(recv_param.find("#") + 1);
            std::string recv_name = recv_name_list.substr(0, recv_name_list.find("#"));
            std::string recv_list = recv_name_list.substr(recv_name_list.find("#") + 1);
            m_queue = new holdback_queue(atoi(recv_init_seq.c_str()), this);
            initMemberList(recv_list, to_addr.getAddress());
            if (!isSureLeaderAddr) member_node->leaderEntry = new MemberListEntry(join_addr, recv_name);

        } else {
            return FAILURE;
        }
    }
    
    member_node->inGroup = true;
    return SUCCESS;
}

void DNode::addMessage(std::string msg) {
    message_chat_queue.push_back(msg);
}


//////////////////////////////// SEND MSG FUNC ////////////////////////////////

/**
 * FUNCTION NAME: sendMsg
 *
 * DESCRIPTION: Send a chat message
 *              If the current node is the leader, call multicast directly
 *              There is a thread continuing calling this method
 */
void DNode::sendMsg(std::string msg) {
    
    std::string leader_address = member_node->getLeaderAddress();
    std::string self_address = member_node->address->getAddress();
    
    if (leader_address.compare(self_address) == 0) { // I'm the leader (sequencer)
        std::string msgToSend = self_address+"#"+username + "::" + msg;
        multicastMsg(msgToSend, D_M_MSG);
    } else { // Send Multicast request to the sequencer
        message_table[seq] = msg;
        message_send_queue.push_back(std::make_pair(seq, msg));
        seq++;
    }
    
}

/**
 * FUNCTION NAME: sendMsgToLeader
 *
 * DESCRIPTION: If election in process Need to take care msgs and send to the new leader
 *              No request will be send to leader if election in process
 */
void DNode::sendMsgToLeader() {
    
    auto msg_pair = message_send_queue.pop();
    
    std::unique_lock<std::mutex> lk(mutex_election); // message with port
    // Wait for election to be finished
    while (getElectionStatus() != E_NONE) {
        d_condition.wait(lk);
    }
    
    if (member_node->getLeaderAddress().compare(member_node->getAddress()) == 0) {
        
        // I'm the leader now
        std::string msgToSend = member_node->getAddress()+"#"+username + ":: " + msg_pair.second;
        multicastMsg(msgToSend, D_M_MSG);
        
    } else {
        
        std::string str_to = std::string(D_CHAT) + "#" + std::to_string(member_node->address->port) +"#" + std::to_string(msg_pair.first) + "#"
        + username + ":: " + msg_pair.second;
        std::string leader_address = member_node->getLeaderAddress();
        std::string str_ack;
        Address leader_addr(leader_address);
        std::cout << "sendMsgToLeader: " << leader_address << std::endl;
        if (dNet->DNsend(&leader_addr, str_to, str_ack, 1) == SUCCESS) {
            
            // If the response is not OK, then it is a number that the leader current seen
            std::string ok("OK");
            if (strcmp(str_ack.c_str(), ok.c_str()) != 0) {
                std::cout << str_to << " ACK from leader: " << str_ack << std::endl;
                int ack = stoi(str_ack);
                // if the ack seq number is less seq - 1, we need to resend that message
                while (ack + 1 < seq) {
                    std::string resend_ack;
                    std::string resend_str = std::string(D_CHAT) + "#" + std::to_string(member_node->address->port) + "#" + std::to_string(ack) + "#"
                    + username + ":: " + message_table[ack];
                    std::cout << "Resend " + resend_str << std::endl;
                    dNet->DNsend(&leader_addr, resend_str, resend_ack, 1);
                    ack++;
                }
            }
            
        } else {
            
            // We think the leader is not responding
            // However we are not sure if the leader has multicast this message or not
            message_send_queue.push_front(msg_pair);
            lk.unlock();
            std::chrono::milliseconds sleepTime(1000);
            std::this_thread::sleep_for(sleepTime);
            
        }
        
    }
    
}

/**
 * FUNCTION NAME: multicastMsg
 *
 * DESCRIPTION: Multicast the message to all the members
 *              TODO: only leaders can call this function
 */
void DNode::multicastMsg(std::string msg, std::string type) {
        
    int seq = m_queue->getNextSequence();

    std::string send_msg =  "#" + type + "#" + std::to_string(seq) + "#" + msg;
    // Send to group members
    auto list = member_node->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        Address addr((*iter).getAddress());
        std::string ack;
        if (dNet->DNsend(&addr, send_msg, ack, 2) == FAILURE) {
#ifdef DEBUGLOG
            std::cout << "\tmulticastMsg: Fail! " << addr.getAddress() << std::endl;
#endif
            // Remove first then recursively multicastMsg
            member_node->deleteMember((*iter).getAddress());
            multicastMsg(iter->getAddress(), D_LEAVEANNO);
        }
    }
    // Send to self
    m_queue->push(std::make_pair(seq, "#"+ type + "#" + msg));
    m_queue->pop();
}


/**
 * FUNCTION NAME: sendNotice (send non-chat, notice message without sequence num)
 *
 * DESCRIPTION: Send a single notice
 */
int DNode::sendNotice(std::string notice, std::string destAddress) {
    Address addr(destAddress);
    std::string str_ack;
    
    if (dNet->DNsend(&addr, notice, str_ack, 1) == FAILURE) {
        return FAILURE;
        
    } else {
        return SUCCESS;
        
    }
}


/**
 * FUNCTION NAME: multicastNotice (multicast non-chat message without sequence num)
 *
 * DESCRIPTION: Multicast a notice to all the members
 *              TODO: only leaders can call this function
 */
void DNode::multicastNotice(std::string notice) {
    
    auto list = member_node->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        Address addr((*iter).getAddress());
        std::string str_ack;
        if (dNet->DNsend(&addr, notice, str_ack, 2) == FAILURE) {
        }
    }
}


//////////////////////////////// LEADER ELECTION ////////////////////////////////

/**
 * FUNCTION NAME: startElection, called by a member
 *
 * DESCRIPTION: member start a election
 */
void DNode::startElection() {
    
    if(getElectionStatus() != E_NONE) {
        return; // In election already
    }
    
    std::unique_lock<std::mutex> lk(mutex_election);

    std::cout << ">>>>> Start Leader Election <<<<<" << std::endl;
    
    // wait for some ANSWER, sleep? RECV in another thread so we are okay?
    updateElectionStatus(E_WAITANS);
    
    // Send D_ELECTION to all other processes with higher IDs, expecting D_ANSWER
    auto list = member_node->memberList;
    int send_count = 0;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        if(iter->getAddress().compare(member_node->getAddress()) > 0) {
            int status = sendNotice(std::string(D_ELECTION) + "#" + member_node->getAddress(), iter->getAddress());
            std::cout << "Send Election to " << iter->getAddress() << " : " << status << std::endl;
            if (status == SUCCESS) {
                send_count++;
            }
        }
    }
    // Send Election to leader as well
    if (member_node->getLeaderAddress().compare(member_node->getAddress()) > 0) {
        int status = sendNotice(std::string(D_ELECTION) + "#" + member_node->getAddress(), member_node->getLeaderAddress());
        std::cout << "Send Election to " << member_node->getLeaderAddress() << " [status] " << status << std::endl;
        if (status == SUCCESS) {
            send_count++;
        }
    }

    // If it knows its id is the highest, it elect itself as coordinator
    // Then sends a Coordinator message to all processes with lower identifier. Election is complete
    if (send_count == 0) {
        updateElectionStatus(E_NONE);
        // Delete your own address from the member list
        member_node->deleteMember(member_node->getAddress());
        // Update leader address
        member_node->updateLeader(*(member_node->address), username);
        // Multicast COOR message to the rest of nodes
        multicastNotice(std::string(D_COOR) + "#" + username + "#" + member_node->getAddress());
        
        m_queue->resetSequence();
        return;
    }
    
    // Wait for COOR message ..........
    updateElectionStatus(E_WAITCOOR);
    lk.unlock();

    std::cout << "Wait for COOR message .........." << std::endl;
    std::chrono::milliseconds sleepTime(COORTIMEOUT);
    std::this_thread::sleep_for(sleepTime);
    
    if(getElectionStatus() == E_WAITCOOR) {
        std::cout << "No COOR message .........." << std::endl;
        updateElectionStatus(E_NONE);
        startElection();
    }
    
}


///////////////////////////////////// HEARTBEAT FUNC /////////////////////////////////////


/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				member: start election; leader: multicast LEAVEANNO
 */
void DNode::nodeLoopOps() {
    
    std::unique_lock<std::mutex> lk(mutex_election);
    
    std::string leader_address = member_node->getLeaderAddress();
    std::string self_address = member_node->address->getAddress();

    if(leader_address.compare(self_address) == 0) { // I am the leader
        
        // have the leader broadcast a heartbeat
        // multicastNotice(std::string(D_HEARTBEAT) + "#" + self_address);
        
        // check every one's heartbeat in the memberlist (except myself)
        auto list = member_node->memberList;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            
            std::string memberAddr = iter->getAddress();
            if(memberAddr.compare(self_address) != 0) {
                
                // check heartbeat
                // if(checkHeartbeat(memberAddr) == FAILURE) {
                if (sendNotice(std::string(D_HEARTBEAT) + "#" + self_address, memberAddr) == FAILURE) {
                    std::cout << "!!! checkHeartbeat fail " << memberAddr << std::endl;
                    // exceed timeout limit
                    deleteMember(memberAddr);
                    // std::string message_leave = memberAddr;
                    multicastMsg(memberAddr, D_LEAVEANNO);
                }
            } else {
                 std::cout << "!!! Never here in nodeLoopOps " << memberAddr << std::endl;
            }
        }
    } else {

        // I am a member and I send heartbeat to leader
        std::string leader_address = member_node->getLeaderAddress();
        std::string self_address = member_node->getAddress();
        
        // check leader heartbeat
//        if(checkHeartbeat(leader_address) == FAILURE) {
        
        if (sendNotice(std::string(D_HEARTBEAT) + "#" + self_address, leader_address) == FAILURE) {

            lk.unlock();
            startElection();
            
        }
    }

    // finish heartbeat check
    return;
}

//////////////////////////////// SNAPSHOT ////////////////////////////////
/**
 * FUNCTION NAME: startSnapshotByUser
 *
 * DESCRIPTION: start taking snapshot, only get called through stdin
 */
void DNode::startSnapshotByUser() {
    std::unique_lock<std::mutex> lk(mutex_snapshot);
    if (getSnapshotStatus() == S_NONE) {
        // initialize Snapshot object
        snapshot = new Snapshot(this->getUsername(), member_node->getAddress(), this);
        
        snapshotCnt++;
        
        // update snapshot status
        updateSnapshotStatus(S_RECORDING);
        
        // record process state
        snapshot->recordState(this);
        
        // send out markers
        std::string str(D_MARKER+std::string("#")+member_node->getAddress());
        multicastMarker(str);
        
        if (snapshot->getChannelNum() == 0) {
            std::cout << "Snapshot Complete!" << std::endl;
            updateSnapshotStatus(S_NONE);
        }
        
        // start record channel messages
    }
    else {  // already recording
        std::cerr << "Error: In process of taking snapshot! Try later!" << std::endl;
        return;
    }
}

/**
 * FUNCTION NAME: startSnapshotByMarker
 *
 * DESCRIPTION: start taking snapshot, only get called through 
                receiving marker from other processes
                When getting called, snapshot_status must be S_NONE
                Suppose received marker from channel ck
 */
Snapshot* DNode::startSnapshotByMarker(std::string from_addr) {
    std::unique_lock<std::mutex> lk(mutex_snapshot);
    // initialize Snapshot object
    snapshot = new Snapshot(this->getUsername(), member_node->getAddress(), this);
    
    snapshotCnt++;
    
    // update snapshot status
    updateSnapshotStatus(S_RECORDING);
    
    // record process state
    snapshot->recordState(this);

    // record from where received the marker
    snapshot->setMarkerFromAddr(from_addr);

    // empty channel ck
    Channel ck = *(snapshot->getChannel(from_addr));
    ck.clearChannel();

    // record marker from which channel
    snapshot->recordChannelMarker(from_addr);

    // send out markers
    std::string str(D_MARKER+std::string("#")+member_node->getAddress());
    multicastMarker(str);
    
    if (snapshot->getChannelNum() == 0) {
        std::cout << "Snapshot Complete!" << std::endl;
        updateSnapshotStatus(S_NONE);
    }
    // start record channel messages
    return snapshot;
}

/**
 * FUNCTION NAME: multicastMarker
 *
 * DESCRIPTION: Multicast a maker to all the members
 */
void DNode::multicastMarker(std::string marker) {
    std::string leader_address = member_node->getLeaderAddress();
    std::string self_address = member_node->getAddress();
    
#ifdef CHANNELDEMO
    std::chrono::milliseconds sleepTime(2500);
    std::this_thread::sleep_for(sleepTime);
#endif
    
    if(leader_address.compare(self_address) == 0) { // I am the leader
        // send marker to everyone except myself
        auto list = member_node->memberList;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            std::string memberAddr = iter->getAddress();
            if(memberAddr.compare(self_address) != 0) {
                if (sendNotice(std::string(D_MARKER) + "#" + self_address, memberAddr) == FAILURE) {
                    std::cout << "fail to send marker to: " << memberAddr << std::endl;
                }
            } else {
                std::cout << "!!! Never here in send out markers " << memberAddr << std::endl;
            }
        }
    } else {
        // I am a member and I send marker to leader and everyone else
        if (sendNotice(std::string(D_MARKER) + "#" + self_address, leader_address) == FAILURE) {
            std::cout << "fail to send marker to leader: " << leader_address << std::endl;
        }
        // send marker to everyone except myself
        auto list = member_node->memberList;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            std::string memberAddr = iter->getAddress();
            if(memberAddr.compare(self_address) != 0) {
                if (sendNotice(std::string(D_MARKER) + "#" + self_address, memberAddr) == FAILURE) {
                    std::cout << "fail to send marker to: " << memberAddr << std::endl;
                }
            }
        }
    }
}

/**
 * update election status
 */
void DNode::updateSnapshotStatus(int new_status) {
    snapshot_status = new_status;
}

void DNode::recordSnapshotChannelMsg(std::string from_addr, std::string msg) {
    std::string marker_addr = snapshot->getMarkerFormAddress();
    
    // received msg from cki, write msg to marker_from_channel
    if (from_addr.compare(marker_addr) == 0) {
        Channel* ck = snapshot->getMarkerFromChannel();
        ck->addMsg(msg);
    }
    // msg from other channels, directly record
    else {
        Channel* ck = snapshot->getChannel(from_addr);
        ck->addMsg(msg);
    }
}

/***
 * static function for writing checkpoint for current node
 ***/
void doWriting(Snapshot* snapshot, Member* member_node, int idx) {
    std::ofstream outfile;
    std::string fileName("Checkpoint_" + member_node->getAddress() + "_" + std::to_string(idx) + ".txt");
    outfile.open (fileName);
    
    // output state
    outfile << "Process state: \n";
    Member* snapshot_member = snapshot->getMember();
    DNode* snapshot_node = snapshot->getNode();
    std::unordered_map<std::string, Channel> snapshot_channels = snapshot->getChannels();
    
    // election status
    outfile << "Election status: " << snapshot_node->getElectionStatus() << "\n\n";
    
    // sequence seen
    int x = snapshot_node->getMQueueSequenceSeen();
    outfile << "Sequence seen: " << x << "\n\n";
    
    // leader information
    outfile << "Leader name: " << snapshot_member->getLeaderName() << "\n";
    outfile << "Leader address: " << snapshot_member->getLeaderAddress() << "\n\n";
    
    // member information
    outfile << "Member list: \n";
    auto list = snapshot_member->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        char buff[20];
        time_t t = snapshot_member->getHeartBeat(iter->getAddress());
        strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
        outfile << "Username: " << iter->getUsername() << ", address: " << iter->getAddress()
        << ", last contact time: " << buff << "\n";
    }
    outfile << "\n";
    
    // output incoming messages:
    outfile << "Channels: \n";
    std::unordered_map<std::string, Channel> temp_c = snapshot->getChannels();
    for ( auto it = snapshot_channels.begin(); it != snapshot_channels.end(); ++it) {
        outfile << "Address: " << it->first << "\nMessages: \n";
        Channel ch = it->second;
        int size = ch.getMsgCnt();
        for (int i = 0; i < size; i++) {
            outfile << ch.retrieveMsg() << "\n";
        }
    }
    outfile << "\n";
    snapshot->setChannels(temp_c);
    
    outfile << "Message history: \n";
    std::deque<std::pair<time_t, std::string>> temp_q = snapshot->getMsgQueue();
    while (!temp_q.empty()) {
        std::pair<time_t, std::string> pair = temp_q.front();
        temp_q.pop_front();
        char buff[20];
        time_t t = pair.first;
        strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
        outfile << buff << ": \n\t" << pair.second << std::endl;
    }
    outfile << "\n";
    
    outfile.close();
    std::cout << "---- Write checkpoint to disk done ----\n";
}

/**
 * FUNCTION NAME: writeCheckpoint
 *
 * DESCRIPTION: start a thread to do disk write
                write snapshot to file
 */
void DNode::writeCheckpoint() {
    std::thread writethread(doWriting, snapshot, member_node, getSnapshotCnt());
    writethread.detach();
}

/**
 * FUNCTION NAME: showSnapshot
 *
 * DESCRIPTION: check whether there is snapshot to be displayed
                if so, call method to display snapshot to stdout
 */
void DNode::showSnapshot() {
    if (getSnapshotCnt() == 0) {
        std::cout << "No snapshot" << std::endl;
    }
    else if (getSnapshotStatus() == S_RECORDING) {
        std::cout << "Taking snapshot now! Try later!" << std::endl;
    }
    else {
        doDisplaySnapshot();
    }
}

/**
 * FUNCTION NAME: doDisplaySnapshot
 *
 * DESCRIPTION: print process state, channel messages to stdout
                only prints most recent snapshot
 */
void DNode::doDisplaySnapshot() {
    // output state
    std::cout << "Process state: \n";
    Member* snapshot_member = snapshot->getMember();
    DNode* snapshot_node = snapshot->getNode();
    std::unordered_map<std::string, Channel> snapshot_channels = snapshot->getChannels();
    
    // election status
    std::cout << "Election status: " << snapshot_node->getElectionStatus() << "\n\n";
    
    // sequence seen
    int x = snapshot_node->getMQueueSequenceSeen();
    std::cout << "Sequence seen: " << x << "\n\n";
    
    // leader information
    std::cout << "Leader name: " << snapshot_member->getLeaderName() << "\n";
    std::cout << "Leader address: " << snapshot_member->getLeaderAddress() << "\n\n";
    
    // member information
    std::cout << "Member list: \n";
    auto list = snapshot_member->memberList;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        char buff[20];
        time_t t = snapshot_member->getHeartBeat(iter->getAddress());
        strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
        std::cout << "Username: " << iter->getUsername() << ", address: " << iter->getAddress()
        << ", last contact time: " << buff << "\n";
    }
    std::cout << "\n";
    
    // output incoming messages:
    
    std::cout << "Channels: \n";
    std::unordered_map<std::string, Channel> temp_c = snapshot->getChannels();
    for ( auto it = snapshot_channels.begin(); it != snapshot_channels.end(); ++it) {
        std::cout << "Address: " << it->first << "\nMessages: \n";
        Channel ch = it->second;
        int size = ch.getMsgCnt();
        for (int i = 0; i < size; i++) {
            std::cout << ch.retrieveMsg() << "\n";
        }
    }
    std::cout << "\n";
    snapshot->setChannels(temp_c);
    
    std::cout << "Message history: \n";
    std::deque<std::pair<time_t, std::string>> temp_q = snapshot->getMsgQueue();
    while (!temp_q.empty()) {
        std::pair<time_t, std::string> pair = temp_q.front();
        temp_q.pop_front();
        char buff[20];
        time_t t = pair.first;
        strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
        std::cout << buff << ": \n\t" << pair.second << std::endl;
    }
    std::cout << "\n";
    std::cout << "End of snapshot.\n";
    std::cout << "---------------------------------------\n";
}


//////////////////////////////// MESSAGE QUEUES //////////////////////////
/**
 * add item to rear of display_msg_queue
 */
void DNode::addDisplayMsg(std::string msg) {
    time_t  now;
    time(&now);
    std::pair<time_t, std::string> p = std::make_pair(now, msg);
    display_msg_queue.push_back(p);
}

/**
 * get first item form display_msg queue
 */
std::pair<time_t, std::string> DNode::peekDisplayMsg() {
    if (!display_msg_queue.empty())
        return display_msg_queue.front();
    else {
        time_t now;
        time(&now);
        std::pair<time_t, std::string> p(now, "");
        return p;
    }
}

/**
 * pop from display msg queue
 */
std::pair<time_t, std::string> DNode::popDisplayMsg() {
    if (!isDisplayMsgQueueEmpty()) {
        std::pair<time_t, std::string> p(peekDisplayMsg());
        display_msg_queue.pop_front();
        return p;
    }
    else {
        time_t now;
        time(&now);
        std::pair<time_t, std::string> p(now, "");
        return p;
    }
}

/**
 * check whether display_msg_queue is empty
 */
bool DNode::isDisplayMsgQueueEmpty() {
    return display_msg_queue.empty();
}

/**
 * display_msg_queue getter
 */
std::deque<std::pair<time_t, std::string>> DNode::getDisplayMessageQueue() {
    return display_msg_queue;
}

/**
 * display_msg_queue setter
 */
void DNode::setDisplayMessageQueue(std::deque<std::pair<time_t, std::string>> q) {
    display_msg_queue = q;
}

//////////////////////////////// GETTERS & SETTERS ////////////////////////////////

/**
 * dNet getter
 */
DNet * DNode::getDNet() {
    return dNet;
}

/**
 * member_node getter
 */
Member * DNode::getMember() {
    return member_node;
}

/**
 * username getter
 */
std::string DNode::getUsername() {
    return username;
}

/**
 * election status getter
 */
int DNode::getElectionStatus() {
    mtx.lock();
    int toReturn = election_status;
    mtx.unlock();
    return toReturn;
}

/**
* push to m_queue
*/
void DNode::pushMQueue(std::pair<int, std::string> value) {
    m_queue->push(value);
}

/**
* sequence number setter
*/
void DNode::setMQueueSequenceSeen(int i) {
    m_queue->setSequenceSeen(i);
}

/**
 * next sequence number getter
 */
void DNode::setMQueueSequenceNext(int i) {
    m_queue->setSequenceNext(i);
}

/**
 * sequence number getter
 */
int DNode::getMQueueSequenceSeen() {
    return m_queue->getSequenceSeen();
}

/**
 * node snapshot getter
 */
Snapshot* DNode::getSnapshot() {
    return snapshot;
}

/**
 * node snapshot setter
 */
void DNode::setSnapshot(Snapshot* s) {
    snapshot = s;
}

/**
 * m_queue_setter
 */
void DNode::setMQueue(holdback_queue q) {
    m_queue = &q;
}

/**
 * pop from message_chat_queue
 */
std::string DNode::popMessageChatQueue() {
    return message_chat_queue.pop();
}

/**
* get first message from m_queue
*/
std::pair<int, std::string> DNode::peekMQueue() {
    return m_queue->peek();
}

/**
 * snapshot status getter
 */
int DNode::getSnapshotStatus() {
    int toReturn = snapshot_status;
    return toReturn;
}

/**
 * snapshot count getter
 */
int DNode::getSnapshotCnt() {
    return snapshotCnt;
}

/**
 * update election status
 */
void DNode::updateElectionStatus(int new_status) {

    mtx.lock();
    election_status = new_status;
    mtx.unlock();

}
