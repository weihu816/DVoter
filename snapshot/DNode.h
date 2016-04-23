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
#include "Snapshot.h"

class Handler;

/**
 * CLASS NAME: DNode
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class DNode {
private:
    
    Member * member_node;
    DNet * dNet;
    std::string join_address;
    std::string username;
    int election_status = E_NONE; // not in election
    int snapshot_status = S_NONE;   // not taking snapshot
    blocking_queue<std::string> message_chat_queue;
    blocking_queue<std::string> message_send_queue;
    Snapshot *snapshot;
    int snapshotCnt;
    std::deque<std::pair<time_t, std::string>> display_msg_queue;

public:
    // multicst_queue will be initilized using a sequence number init_seen from the leader
    holdback_queue * m_queue;
    std::mutex mtx;
    std::mutex mutex_election;
    std::mutex mutex_snapshot;
//    std::condition_variable cv;

    DNode(std::string name, std::string join_addr="");
    int nodeStart();                                                // introduce and start functions
    int initThisNode();                                             // parameter initialization
    int introduceSelfToGroup(std::string joinAddress, bool isSureLeaderAddr);
    int nodeLeave();                                                // Wind up this node and clean up state
    void initMemberList(std::string member_list, std::string leaderAddr);
    void addMember(std::string ip_port, std::string name, bool toPrint);
    void deleteMember(std::string ip_port);                         // delete a member
    void clearMembers();                                            // delete all members
    
    int recvLoop();
    std::string msgLoop();
    void nodeLoop();
    
    static int enqueueWrapper(void *env, char *buff, int size);
    void nodeLoopOps(); // this is the operation for heartbeat
    int checkHeartbeat(std::string address);
    
    void startElection();

    void addMessage(std::string msg);
    void recvHandler(std::pair<Address, std::string>);
    
    void sendMsg(std::string msg);
    void sendMsgToLeader();
    void multicastMsg(std::string msg, std::string type);
    int sendNotice(std::string type, std::string addr);
    void multicastNotice(std::string type);
    
    Member* getMember();
    DNet* getDNet();
    std::string getUsername();
    int getElectionStatus();
    void updateElectionStatus(int new_status);

    void startSnapshotByUser();
    Snapshot* startSnapshotByMarker(std::string from_addr);
    int getMessageChatQueueSize();
    int getMessageSendQueueSize();
    int getMQueueSize();
    void pushMessageChatQueue(std::string);
    void pushMessageSendQueue(std::string);
    void pushMQueue(std::pair<int, std::string> value);
    void setMQueueSequenceSeen(int i);
    void setMQueueSequenceNext(int i);
    int getMQueueSequenceSeen();
    int getMQueueSequenceNext();
    void setMessageChatQueue(blocking_queue<std::string> q);
    void setMessageSendQueue(blocking_queue<std::string> q);
    void setMQueue(holdback_queue q);
    std::string popMessageChatQueue();
    std::string popMessageSendQueue();
    std::pair<int, std::string> popMQueue();
    std::string peekMessageChatQueue();
    std::string peekMessageSendQueue();
    std::pair<int, std::string> peekMQueue();
    blocking_queue<std::string> getMessageChatQueue();
    blocking_queue<std::string> getMessageSendQueue();
    holdback_queue getMQueue();
    
    void multicastMarker(std::string marker);
    int getSnapshotStatus();
    void updateSnapshotStatus(int new_status);
    Snapshot* getSnapshot();
    void setSnapshot(Snapshot* s);
    void recordSnapshotChannelMsg(std::string from_addr, std::string msg);
    void writeCheckpoint();
    int getSnapshotCnt();
    void showSnapshot();
    void doDisplaySnapshot();
    
    void addDisplayMsg(std::string msg);
    std::pair<time_t, std::string> peekDisplayMsg();
    std::pair<time_t, std::string> popDisplayMsg();
    bool isDisplayMsgQueueEmpty();
    std::deque<std::pair<time_t, std::string>> getDisplayMessageQueue();
    void setDisplayMessageQueue(std::deque<std::pair<time_t, std::string>> q);
    
    virtual ~DNode() {
        if(dNet) delete dNet;
        if(member_node) delete member_node;
        if (m_queue) delete m_queue;
        if (snapshot) delete snapshot;
    }
};

#endif /* DNODE_H */