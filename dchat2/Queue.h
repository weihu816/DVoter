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


class queue_object {
public:
    
    int key; // logical local timestamp
    int id; // Final timestamp is determined by id and pid
    int pid;
    std::string content; // Raw content of message
    bool isDeliverable; // deliverable or not
    
    queue_object() : isDeliverable(false) {
        
    }
    
    queue_object(int key, std::string content) : key(key), content(content), isDeliverable(false) {

    }

    queue_object(const queue_object &another_queue_object) {
        this->id = another_queue_object.id;
        this->pid = another_queue_object.pid;
        this->key = another_queue_object.key;
        this->content = another_queue_object.content;
    }
    
    queue_object& operator=(const queue_object& another_queue_object) {
        this->id = another_queue_object.id;
        this->pid = another_queue_object.pid;
        this->key = another_queue_object.key;
        this->content = another_queue_object.content;
        return *this;
    }
};


class blocking_priority_queue {
private:
    class cmp {
    public:
        cmp() {}
        // Ordered by ascending order of timestamp - key
        bool operator() (const queue_object & a, const queue_object & b) const {
            return a.key > b.key;
        }
    };
    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::priority_queue<queue_object, std::vector<queue_object>, cmp> d_queue;
    
public:
    blocking_priority_queue() {}
    blocking_priority_queue(const blocking_priority_queue&) = delete;
    blocking_priority_queue& operator=(const blocking_priority_queue&) = delete;

    void push(queue_object const& value) {
        std::unique_lock<std::mutex> mlock(d_mutex);
        d_queue.push(value);
        d_condition.notify_one();
    }

    queue_object pop() {
        std::unique_lock<std::mutex> mlock(d_mutex);
        while (d_queue.empty()) {
            d_condition.wait(mlock);
        }
        queue_object rc = d_queue.top();
        d_queue.pop();
        return rc;
    }
    
    void makeDeliverable() {
        std::unique_lock<std::mutex> mlock(d_mutex);
        if (d_queue.empty()) return;
        for (auto i = d_queue.size()) {
            
        }
        queue_object rc = d_queue.top();
        d_queue.pop();
        return rc;
    }
};

#endif /* BLOCKINGQUEUE_H */
