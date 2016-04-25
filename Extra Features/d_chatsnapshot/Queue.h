/**********************************
 * FILE NAME: BlockingQueue.h
 *
 * DESCRIPTION: BlockingQueue classes header file
 **********************************/

#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include "stdincludes.h"
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
    void push_back(T const& value) {
        std::unique_lock<std::mutex> mlock(d_mutex);
        d_queue.push_back(value);
        d_condition.notify_one();
    }
    void push_front(T const& value) {
        std::unique_lock<std::mutex> mlock(d_mutex);
        d_queue.push_front(value);
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
    long size() {
        std::unique_lock<std::mutex> mlock(d_mutex);
        return d_queue.size();
    }
    void clear() {
        std::unique_lock<std::mutex> mlock(d_mutex);
        d_queue.clear();
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
    T peek();
    void handle(std::string msg);
    int getNextSequence();
    void resetSequence();
    int getHoldBackQueueSize();
    void setSequenceSeen(int i);
    void setSequenceNext(int i);
    void insert(std::pair<int, std::string> v);
};

class blocking_priority_queue {
private:
    class cmp {
    public:
        cmp() {}
        bool operator() (const T & a, const T & b) const { return a.first < b.first; } // Descending
    };
    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::priority_queue<T, std::vector<T>, cmp> d_queue;
    
public:
    blocking_priority_queue() {}
    blocking_priority_queue(const blocking_priority_queue&) = delete;
    blocking_priority_queue& operator=(const blocking_priority_queue&) = delete;
    
    void push(T const& value) {
        std::unique_lock<std::mutex> mlock(d_mutex);
        d_queue.push(value);
        d_condition.notify_one();
    }
    
    T pop() {
        std::unique_lock<std::mutex> mlock(d_mutex);
        while (d_queue.empty()) {
            d_condition.wait(mlock);
        }
        T rc = d_queue.top();
        d_queue.pop();
        return rc;
    }
};

#endif /* BLOCKINGQUEUE_H */
