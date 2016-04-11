/**********************************
 * FILE NAME: Handler.cpp
 *
 * DESCRIPTION: Handler class
 **********************************/

#include "Handler.h"
#include "DNode.h"

using namespace std;

vector<string> splitstr(string ori, char deli) {
    vector<string> tmp;
    stringstream ss(ori);
    string tok;
    while (getline(ss, tok, deli)) {
        tmp.push_back(tok);
    }
    return tmp;
}

/**
 * FUNCTION NAME: electionHandler
 *
 * DESCRIPTION: handle election: start if no message recved in time
 */
void electionHandler(DNode * node) {
    // waits some time for D_COOR
    node->updateElectionStatus(E_WAITCOOR);
    std::chrono::milliseconds sleepTime(ELECTIONTIME);
    std::this_thread::sleep_for(sleepTime);
    
    if(node->getElectionStatus() != E_NONE) {// recv
        // If it does not receive this message in time, it re-broadcasts the D_ELECTION
        node->startElection();
    }
    
}


string Handler::process(Address & from_addr, string recv_msg) {
    
#ifdef DEBUGLOG
    std::cout << "\tHandling message: " + recv_msg << " from: " << from_addr.getAddress() << std::endl;
#endif
    
    std::string send_msg;
    if (recv_msg.empty()) return "";
    
    char * cstr = new char[recv_msg.length() + 1];
    strcpy(cstr, recv_msg.c_str());
    char * msg_type = strtok(cstr, "#");

#ifdef DEBUGLOG
    std::cout << "\tMessage Type: " << msg_type << std::endl;
#endif
    
    Member * nodeMember = node->getMember();
    
    if (recv_msg.find("#") == 0) { // Start with a #
        
        if (strcmp(msg_type, D_M_ADDNODE) == 0) {
            
            // received: #ADDNODE#SEQ#ip#port#name
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_ip(strtok(NULL, "#"));
            std::string param_port(strtok(NULL, "#"));
            std::string param_name(strtok(NULL, "#"));
            std::string param_value(param_ip + "#" + param_port + "#" + param_name);
            node->m_queue->push(std::make_pair(param_seq, "#"+std::string(msg_type)+"#"+param_value));
            node->m_queue->pop();
            return "OK";

        } else if (strcmp(msg_type, D_M_MSG) == 0) {
            
            // received: #MSG#SEQ#username::Message
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_value(strtok(NULL, "#"));
            node->m_queue->push(std::make_pair(param_seq, "#"+std::string(msg_type)+"#"+param_value));
            //node->addMessage(param_value);
            node->m_queue->pop();
            return "OK";
        } else if (strcmp(msg_type, D_LEAVEANNO) == 0) {
            // TODO: received: #LEAVEANNO#seq#name#ip:port sent by leader
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_name_addr(strtok(NULL, "#"));
            node->m_queue->push(std::make_pair(param_seq, "#"+std::string(msg_type)+"#"+param_name_addr));
            node->m_queue->pop();
            
            return "OK";
        }
        
    } else {
        
        if (strcmp(msg_type, D_CHAT) == 0) {

            // received: CHAT#Message - From node to sequencer
            std::string recv_msg(strtok (NULL, "#"));
            if (nodeMember->isLeader()) { // Only leader can multicast messages
                // msg to be sent: #MSG#SEQ#username#Message
                std::string message = recv_msg;
                node->multicastMsg(message, D_M_MSG);
            }
            return "OK";
            
        } else if (strcmp(msg_type, D_JOINREQ) == 0) {

            if (nodeMember->isLeader()) { // I am the leader

                // received: JOINREQ#PORT#name
                // First need to add this member to the list (should not exist)
                // If it's a multi-threaded server, seq number should be sync with other message handling
                std::string recv_port(strtok (NULL, "#"));
                std::string recv_name(strtok (NULL, "#"));

                // #ADDNODE#SEQ#ip#port#name, multicast addnode message from the sequencer
                // This message must be delivered once (at least onece)
                std::string message_addmember = from_addr.getAddressIp() + "#" + recv_port + "#" + recv_name;

                // Start a thread to do the multicast to avoid blocking self
                node->multicastMsg(message_addmember, D_M_ADDNODE);
                
                std::string member_addr = from_addr.getAddressIp() + ":" + recv_port;
                std::string member_name = recv_name;
                //TODO logic: wait for broadcast ADD_NODE?
                //node->addMember(member_addr, member_name);
                int initSeq = node->m_queue->getSequenceSeen();
                // send JOINLIST#initSeq#ip1:port1:name1:ip2:port2:name2...
                std::string message = std::string(D_JOINLIST) + "#" + std::to_string(initSeq) +
                "#" + node->getUsername() + "#" + nodeMember->getMemberList();
#ifdef DEBUGLOG
                std::cout << "\tHandling Returns: " << message << std::endl;
#endif
                return message;
                
            } else {

                // received: JOINLEADER#LEADERIP#LEADERPORT
                // send back leader address - don't care failure, they can retry
                std::string message = std::string(D_JOINLEADER) + "#" + nodeMember->getLeaderAddressIp()
                + "#" + nodeMember->getLeaderAddressPort();
                return message;

            }
        } else if (strcmp(msg_type, D_LEAVE) == 0) {    //can only be received by leader
            
            // TODO: received: LEAVE#name#ip:port
            std::string leave_name(strtok(NULL, "#"));
            std::string leave_addr(strtok(NULL, "#"));

            // delete leaving node from member_list
            node->deleteMember(leave_addr);
            //D_LEAVE is sent to the leader, and leader should send out LEAVEANNO#seq#name#ip:port
            node->multicastMsg(leave_addr, D_LEAVEANNO);
            return "OK";
            
        } else if (strcmp(msg_type, D_HEARTBEAT) == 0) {
            
            // received: HEARTBEAT#ip:port
            // know node at from_addr is still there, update heartbeat for node at from_addr
            std::string node_addr(strtok(NULL, "#"));
            time_t timev;
            time(&timev);
            nodeMember->updateHeartBeat(node_addr, timev);
#ifdef DEBUGLOG
            std::cout << "HeartBeat updated " << nodeMember->getHeartBeat(node_addr) << std::endl;
#endif
            return "OK";
        } else if (strcmp(msg_type, D_ELECTION) == 0) {
            // TODO: received: ELECTION#ip:port
            std::string heardFrom(strtok(NULL, "#"));
            // If hears D_ELECTION from a process with a higher ID,
            if(heardFrom.compare(nodeMember->getAddress()) > 0) {
#ifdef DEBUGLOG
                std::cout << "\tElection: heard from someone larger... " << heardFrom << std::endl;
#endif
                //pass to handler thread
                std::thread thread_election(electionHandler, node);
                thread_election.detach();
             } else {
#ifdef DEBUGLOG
                 std::cout << "\tElection: heard from someone smaller... " << heardFrom << std::endl;
#endif
                // If hears D_ELECTION from a process with a lower ID
                // send back D_ANSWER and startElection myself
                node->sendNotice(D_ANSWER, heardFrom);
                node->startElection();
            }

            return "OK";
            
        } else if (strcmp(msg_type, D_ANSWER) == 0) {
            
            // TODO: received: ANSWER#ip:port
            std::string heardFrom(strtok(NULL, "#"));
            if(heardFrom.compare(nodeMember->getAddress()) > 0 && node->getElectionStatus() == E_WAITANS) {
                node->updateElectionStatus(E_WAITCOOR);
            }
#ifdef DEBUGLOG
            std::cout << "\tElection: heard an answer, wait for COOR ... " << std::endl;
#endif
            return "OK";
            
        } else if (strcmp(msg_type, D_COOR) == 0) {
            // received: COOR#name#ip:port
            std::string leader_name(strtok(NULL, "#"));
            std::string heardFrom(strtok(NULL, "#"));
            // TODO: received: COOR
            if(node->getElectionStatus() == E_WAITCOOR) {
                node->updateElectionStatus(E_NONE);
                nodeMember->updateLeader(heardFrom, leader_name); // this one displays last leader left
                node->m_queue->resetSequence();
            }
            return "OK";
            
        } else {
#ifdef DEBUGLOG
            std::cout << "\tReceive Unexpected: " << recv_msg << std::endl;
#endif
        }

    }

    return "";
}