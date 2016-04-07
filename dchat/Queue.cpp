#include "Queue.h"
#include "DNode.h"

void holdback_queue::pop() {
    std::unique_lock<std::mutex> mlock(d_mutex);
    while (!d_queue.empty() && d_queue.top().first <= sequence_seen) {
        T peek_pair = d_queue.top();
        d_queue.pop();
        // Handle peek_pair
#ifdef DEBUGLOG
        std::cout << "\t#holdback_queue handling: " << peek_pair.first << " " << peek_pair.second << std::endl;
#endif
        handle(peek_pair.second.substr(1));
    }
    while (!d_queue.empty() && d_queue.top().first == sequence_seen + 1) {
        T peek_pair = d_queue.top();
        d_queue.pop();
        sequence_seen++;
        // Handle peek_pair
#ifdef DEBUGLOG
        std::cout << "\t#tholdback_queue handling: " << peek_pair.first << " " << peek_pair.second << std::endl;
#endif
        handle(peek_pair.second.substr(1));
    }
}

void holdback_queue::handle(std::string msg) {
    std::string msg_type = msg.substr(0, msg.find("#"));
    std::string msg_body = msg.substr(msg.find("#") + 1);
    if (msg_type.compare(D_M_ADDNODE)) {
        // ip#port#name
        std::string ip = msg_body.substr(0, msg_body.find("#"));
        std::string port_name = msg_body.substr(msg_body.find("#") + 1);
        std::string port = port_name.substr(0, port_name.find("#"));
        std::string name = port_name.substr(port_name.find("#") + 1);
        node->addMember(ip + ":" + port, name);
    } else if (msg_type.compare(D_M_MSG)) {
        node->addMessage(msg);
    }
}

void holdback_queue::push(const T &value)  {
#ifdef DEBUGLOG
    std::cout << "\t#holdback_queue push: " << value.first << " " << value.second << std::endl;
#endif
    std::unique_lock<std::mutex> mlock(d_mutex);
    d_queue.push(value);
    d_condition.notify_one();
}

int holdback_queue::getNextSequence()  {
    std::unique_lock<std::mutex> mlock(d_mutex);
    return sequence_next++;
}

holdback_queue::holdback_queue(int init_seen, DNode * node) : node(node) {
    sequence_seen = init_seen;
    sequence_next = init_seen + 1;
}