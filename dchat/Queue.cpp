#include "Queue.h"
#include "DNode.h"


/**
 * FUNCTION NAME: pop
 *
 * DESCRIPTION:
 */
T holdback_queue::pop() {
    std::unique_lock<std::mutex> mlock(d_mutex);
    
    // Dangerous Zone
    while (!d_queue.empty() && d_queue.top().first <= sequence_seen) {
        std::cout << "? This should never happen in holdback_queue" << std::endl;
        d_queue.pop();
    }
    
    // Handling Loop
    while (!d_queue.empty() && d_queue.top().first == sequence_seen + 1) {
        T peek_pair = d_queue.top();
        d_queue.pop();
        sequence_seen++;
        // Handle peek_pair
#ifdef DEBUGLOG
        std::cout << "\t#holdback_queue handling: " << peek_pair.first << " " << peek_pair.second << std::endl;
#endif
        handle(peek_pair.second.substr(1));
        return peek_pair;
    }
}


/**
 * FUNCTION NAME: handle
 *
 * DESCRIPTION:
 */
void holdback_queue::handle(std::string msg) {

    std::string msg_type = msg.substr(0, msg.find("#"));
    std::string msg_body = msg.substr(msg.find("#") + 1);
    
#ifdef DEBUGLOG
    std::cout << "\tQueue handling... " << msg_type << " : " << msg_body << std::endl;
#endif

    if (msg_type.compare(D_M_ADDNODE) == 0) {
        
        // ip#port#name
        std::string ip = msg_body.substr(0, msg_body.find("#"));
        std::string port_name = msg_body.substr(msg_body.find("#") + 1);
        std::string port = port_name.substr(0, port_name.find("#"));
        std::string name = port_name.substr(port_name.find("#") + 1);
        node->addMember(ip + ":" + port, name, true);
        
    } else if (msg_type.compare(D_M_MSG) == 0) {

        node->addMessage(msg_body);

    } else if (msg_type.compare(D_LEAVEANNO) == 0) {
        // name#ip:port
        std::string leader_addr = node->getMember()->getLeaderAddress();
        std::string my_addr = node->getMember()->getAddress();
        if (leader_addr.compare(my_addr) == 0) {     // if i am leader, i have already deleted
            return;
        }
        else {
            // I am a member, i need to delete the leaving member
            // delete the member from the memberList and display message in node function
            std::cout << " === " << msg << std::endl;
            node->deleteMember(msg_body);
        }
    }
}


/**
 * FUNCTION NAME: push
 *
 * DESCRIPTION:
 */
void holdback_queue::push(const T &value)  {
#ifdef DEBUGLOG
    std::cout << "\t#holdback_queue push: " << value.first << " " << value.second << std::endl;
#endif
    std::unique_lock<std::mutex> mlock(d_mutex);
    d_queue.push(value);
    d_condition.notify_one();
}


/**
 * FUNCTION NAME: getNextSequence
 *
 * DESCRIPTION:
 */
int holdback_queue::getNextSequence()  {
    std::unique_lock<std::mutex> mlock(d_mutex);
    return sequence_next++;
}


/**
 * FUNCTION NAME: resetSequence
 *
 * DESCRIPTION:
 */
void holdback_queue::resetSequence() {
    std::unique_lock<std::mutex> mlock(d_mutex);
    sequence_seen = 0;
    sequence_next = 1;
}


/**
 * FUNCTION NAME: holdback_queue
 *
 * DESCRIPTION:
 */
holdback_queue::holdback_queue(int init_seen, DNode * node) : node(node) {
    sequence_seen = init_seen;
    sequence_next = init_seen + 1;
}