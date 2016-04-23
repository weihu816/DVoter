/**********************************
 * FILE NAME: BlockingQueue.h
 *
 * DESCRIPTION: BlockingQueue classes header file
 **********************************/

#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include "stdincludes.h"
#include "Member.h"
#include <mutex>
#include <condition_variable>

class DNode;

template <typename T>
class blocking_queue {
private:
    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::deque<T> d_queue;

public:
    blocking_queue()=default;
    blocking_queue(const blocking_queue&) = delete;
    blocking_queue& operator=(const blocking_queue&) = delete;
    void push(T const& value) {
        std::unique_lock<std::mutex> mlock(d_mutex);
        d_queue.push_back(value);
        d_condition.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> mlock(d_mutex);
        while (d_queue.empty()) {
            d_condition.wait(mlock);
        }
        T rc = d_queue.front();
        d_queue.pop_front();
        return rc;
    }
};

typedef std::pair<int, std::string> T;

class holdback_queue {
private:
    DNode * node;
    class cmp {
    public:
        cmp() {}
        bool operator() (const T & a, const T & b) const { return b.first < a.first; }
    };
    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::priority_queue<T, std::vector<T>, cmp> d_queue;
    int sequence_seen;
    int sequence_next;

public:
    holdback_queue(int init_seen, DNode * node);
    holdback_queue(const holdback_queue&) = delete;
    holdback_queue& operator=(const holdback_queue&) = delete;
    int getSequenceSeen() {return sequence_seen;}
    void push(T const& value);
    void pop();
    void handle(std::string msg);
    int getNextSequence();
    void resetSequence();
};

///////////// Priority ////////////////

class socket_queue_item {
public:
    int priority;
    int sockfd;
    Address from_addr;
    sockaddr_storage their_addr;
    std::string message;

public:
    socket_queue_item(int priority, int sockfd, sockaddr_storage * their_addr, Address fromAddr, std::string message){
        this->priority = priority;
        this->sockfd = sockfd;
        memcpy(&(this->their_addr), their_addr, sizeof(*their_addr));
        this->from_addr = fromAddr;
        this->message = message;
    }
    //Copy Constructor
    socket_queue_item(const socket_queue_item & anotherItem) {
        this->priority = anotherItem.priority;
        this->sockfd = anotherItem.sockfd;
        memcpy(&(this->their_addr), &anotherItem.their_addr, sizeof(anotherItem.their_addr));
        this->from_addr = anotherItem.from_addr;
        this->message = anotherItem.message;
    }
    // Assignment operator overloading
    socket_queue_item& operator =(const socket_queue_item& anotherItem) {
        this->priority = anotherItem.priority;
        this->sockfd = anotherItem.sockfd;
        memcpy(&(this->their_addr), &anotherItem.their_addr, sizeof(anotherItem.their_addr));
        this->from_addr = anotherItem.from_addr;
        this->message = anotherItem.message;
        return *this;
    }
    
};


////////////////////////////////////////

class blocking_priority_queue {
private:
    class cmp {
    public:
        cmp() {}
        bool operator() (const socket_queue_item & a, const socket_queue_item & b) const { return a.priority < b.priority; } // Descending (higher, larger priorty)
    };
    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::priority_queue<socket_queue_item, std::vector<socket_queue_item>, cmp> d_queue;
    
public:
    blocking_priority_queue() {}
    blocking_priority_queue(const blocking_priority_queue&) = delete;
    blocking_priority_queue& operator=(const blocking_priority_queue&) = delete;

    void push(socket_queue_item const& value) {
        std::unique_lock<std::mutex> mlock(d_mutex);
        d_queue.push(value);
        d_condition.notify_one();
    }

    socket_queue_item pop() {
        std::unique_lock<std::mutex> mlock(d_mutex);
        while (d_queue.empty()) {
            d_condition.wait(mlock);
        }
        socket_queue_item rc = d_queue.top();
        d_queue.pop();
        return rc;
    }
};

#endif /* BLOCKINGQUEUE_H */
