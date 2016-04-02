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

#endif /* BLOCKINGQUEUE_H */
