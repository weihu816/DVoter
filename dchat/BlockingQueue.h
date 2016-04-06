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
        mlock.unlock();
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
    class cmp {
    public:
        cmp() {}
        bool operator() (const T & a, const T & b) const { return b.first < a.first; }
    };
    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::priority_queue<T, std::vector<T>, cmp> d_queue;
    int sequence_seen;

public:
    holdback_queue(int init_seen) : sequence_seen(init_seen) { }
    holdback_queue(const holdback_queue&) = delete;
    holdback_queue& operator=(const holdback_queue&) = delete;
    int getSequenceSeen() {return sequence_seen;}
    void push(T const& value) {
        std::unique_lock<std::mutex> mlock(d_mutex);
        mlock.lock();
        d_queue.push(value);
        mlock.unlock();
        d_condition.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> mlock(d_mutex);
        mlock.lock();
        while (!d_queue.empty() && d_queue.top().first <= sequence_seen) {
            d_queue.pop();
        }
        if (d_queue.empty()) return std::make_pair(-1, "");
        T rc = d_queue.top();
        if (rc.first == sequence_seen + 1) {
            d_queue.pop();
            return rc;
        } else {
            return std::make_pair(-1, "");
        }
    }
    
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
        mlock.unlock();
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
