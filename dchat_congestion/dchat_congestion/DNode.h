/**********************************
 * FILE NAME: DNode.h
 *
 * DESCRIPTION: Membership protocol run by this Node
 *              Header file of DNode class.
 **********************************/

#ifndef DNODE_H
#define DNODE_H

#include "stdincludes.h"
#include "Member.h"
#include "DNet.h"
#include "Queue.h"

class Handler;

/**
 * CLASS NAME: DNode
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class DNode {
private:
    
    DNet * dNet;
    Member * member_node;
    
    std::string join_address;
    std::string username;
    
    // Message ready to be displayed
    blocking_queue<std::string> message_chat_queue;
    
    // chat messages related
    int seq = 1; // The seq number to be sent
    std::map<int, std::string> message_table;
    blocking_queue<std::pair<int, std::string>> message_send_queue;
    
    int election_status = E_NONE; // not in election
    std::condition_variable d_condition; // for sending messages

public:
    // traffic congestion
    void leaderTrafficLoop();
    int sleepInt; // the sleep interval in milli sec
    std::mutex traffic_control;
    
    int getSleepInt() {
        std::unique_lock<std::mutex> lock(traffic_control);
        return sleepInt;
    }
    
    void updataSleepInt(std::string traffic_msg) { // alter sleep interval according to traffic control message
        std::unique_lock<std::mutex> lock(traffic_control);
        if(traffic_msg.compare(D_FAST) == 0) {
            sleepInt  = 0;
        } else { // traffic_msg = D_SLOW
            sleepInt = SLOWINT;
        }
    }
    
    // multicst_queue will be initilized using a sequence number init_seen from the leader
    holdback_queue * m_queue;
    std::mutex mtx;
    std::mutex mutex_election;

    DNode(std::string name, std::string join_addr="");
    int nodeStart();                                // introduce and start functions
    int initThisNode();                             // parameter initialization
    int nodeLeave();                                // Wind up this node and clean up state
    
    // Membership
    int introduceSelfToGroup(std::string joinAddress, bool isSureLeaderAddr);
    void initMemberList(std::string member_list);
    void addMember(std::string ip_port, std::string name, bool toPrint);
    void deleteMember(std::string ip_port);
    
    // Routine
    int recvLoop();
    std::string msgLoop();
    void nodeLoop();
    
    
    // Failure detection
    void nodeLoopOps();
    void startElection();
    void updateElectionStatus(int new_status);
    // This is for the leader (sequencer)
    std::map<std::string, int> message_seen;
    // leader traffic counter
    std::map<std::string, int> message_counter_table;

    // Total ordered multicast
    void multicastMsg(std::string msg, std::string type);
    
    // Message Routine
    void addMessage(std::string msg);
    void sendMsg(std::string msg);
    void sendMsgToLeader();
    int sendNotice(std::string type, std::string addr);
    void multicastNotice(std::string type);
    
    // Getters
    Member* getMember();
    std::string getUsername();
    int getElectionStatus();
    
    // Reset seqience number and leader message counter
    // Notify if the message sending is currently blocked
    void resetSeq() {
        message_seen.clear();
        d_condition.notify_all();
        message_table.clear();
        std::deque<std::pair<int, std::string>> temp;
        while (message_send_queue.size() > 0) {
            temp.push_back(message_send_queue.pop());
        }
        message_send_queue.clear();
        seq = 1;
        for (auto pair : temp) {
            message_send_queue.push_back(std::make_pair(seq, pair.second));
            message_table[seq] = pair.second;
            seq++;
        }
    }

    virtual ~DNode() {
        if(dNet) delete dNet;
        if(member_node) delete member_node;
        if (m_queue) delete m_queue;
    }
};

#endif /* DNODE_H */