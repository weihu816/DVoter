/**********************************
 * FILE NAME: BlockingQueue.h
 *
 * DESCRIPTION: BlockingQueue classes header file
 **********************************/

#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include "stdincludes.h"

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
    
    int key;                // logical local timestamp
    int id;                 // Final timestamp is determined by id and pid
    int pid;
    std::string content;    // Raw content of message
    bool isDeliverable;     // deliverable or not
    
    queue_object(int key, std::string content) : key(key), content(content), isDeliverable(false) {

    }

    queue_object(const queue_object &another_queue_object) {
        this->key = another_queue_object.key;
        this->id = another_queue_object.id;
        this->pid = another_queue_object.pid;
        this->content = another_queue_object.content;
        this->isDeliverable = another_queue_object.isDeliverable;
    }
    
    queue_object& operator=(const queue_object& another_queue_object) {
        this->key = another_queue_object.key;
        this->id = another_queue_object.id;
        this->pid = another_queue_object.pid;
        this->content = another_queue_object.content;
        this->isDeliverable = another_queue_object.isDeliverable;
        return *this;
    }
};


class blocking_priority_queue {
private:

    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::deque<queue_object> d_queue;
    
    static bool cmp(const queue_object & a, const queue_object & b) {
        return a.key < b.key;
    }

    DNode * node;
    
public:
    blocking_priority_queue(DNode * node);
    blocking_priority_queue(const blocking_priority_queue&) = delete;
    blocking_priority_queue& operator=(const blocking_priority_queue&) = delete;

    void push(queue_object const& value);
    queue_object pop();
    void makeDeliverable(int k);
};

#endif /* BLOCKINGQUEUE_H */
