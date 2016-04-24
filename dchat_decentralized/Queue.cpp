#include "Queue.h"
#include "DNode.h"

blocking_priority_queue::blocking_priority_queue(DNode * node) : node(node) {
    
}

void blocking_priority_queue::push(queue_object const& value) {
    std::unique_lock<std::mutex> mlock(d_mutex);
    d_queue.push_back(value);
    d_condition.notify_one();
}

queue_object blocking_priority_queue::pop() {
    std::unique_lock<std::mutex> mlock(d_mutex);
    while (d_queue.empty()) {
        d_condition.wait(mlock);
    }
    queue_object rc = d_queue.front();
    d_queue.pop_front();
    return rc;
}

void blocking_priority_queue::makeDeliverable(int k) {

    if (d_queue.empty()) return;
    queue_object * p = nullptr;
    for (auto i = d_queue.begin(); i != d_queue.end(); i++) {
        if ((*i).key == k) {
            (*i).isDeliverable = true;
            p = &(*i);
        }
    }
    if (p == nullptr) return;
    
    std::sort(d_queue.begin(), d_queue.end(), cmp);
    
    auto i = d_queue.begin();
    while (i != d_queue.end()) {
        if ((*i).isDeliverable) {
            std::string content(p->content);
#ifdef INFOLOG
            std::cout << "\t[INFO] Deliverable: " << p->content << std::endl;
#endif
            std::string param_header = content.substr(0, content.find("#"));
            std::string param_body = content.substr(content.find("#") + 1);
            
            if (param_header.compare(D_M_CHAT) == 0) {
                
                node->addMessageToDisplay(param_body);
                
            } else if (param_header.compare(D_M_ADDNODE) == 0) {
                
                // ip#port#name
                std::string param_ip = param_body.substr(0, param_body.find("#"));
                std::string param_port_name = param_body.substr(param_body.find("#") + 1);
                std::string param_port = param_port_name.substr(0, param_port_name.find("#"));
                std::string param_name = param_port_name.substr(param_port_name.find("#") + 1);
                node->addMember(param_ip + ":" + param_port, param_name, true);
                
            } else {
                
                std::cout << "Unexpected message in multicast queue" << std::endl;
                
            }
            
            d_queue.erase(i);
            i = d_queue.begin();
            
        } else {
            break;
        }
    }
}