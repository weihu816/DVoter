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

// when taking snapshot, recording incoming messages
string Handler::process(Address & from_addr, string recv_msg) {
    
#ifdef DEBUGLOG
    if (recv_msg.find(std::string("HEARTBEAT")) == std::string::npos) {
        std::cout << "\tHandling message: " + recv_msg << " from: " << from_addr.getAddress() << std::endl;
    }
#endif
    
    std::string send_msg;
    if (recv_msg.empty()) return "";
    
    char cstr[MAXBUFLEN];
    strcpy(cstr, recv_msg.c_str());
    char * msg_type = strtok(cstr, "#");

#ifdef DEBUGLOG
    if (strcmp(msg_type, "HEARTBEAT") != 0) {
        std::cout << "\tMessage Type: " << msg_type << std::endl;
    }
#endif
    
    Member * nodeMember = node->getMember();
    Snapshot *s = node->getSnapshot();
    
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
            
            // snapshot
            if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
    std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                std::string addr(param_ip+":"+param_port);
                // only record incoming msg from channels other than ck
                if (addr.compare(s->getMarkerFormAddress()) != 0) {
                    Channel *c = s->getChannel(addr);
                    c->addMsg(recv_msg);
                }
                else {
                    Channel *c = s->getMarkerFromChannel();
                    c->addMsg(recv_msg);
                }
            }
            
            return "OK";

        } else if (strcmp(msg_type, D_M_MSG) == 0) {
            
            // received: #MSG#SEQ#addr#username::Message
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_addr(strtok(NULL, "#"));
            std::string param_value(strtok(NULL, "#"));
            node->m_queue->push(std::make_pair(param_seq, "#"+std::string(msg_type)+"#"+param_addr+"#"+param_value));
            //node->addMessage(param_value);
            node->m_queue->pop();
            
            // snapshot
            if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
    std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                // msg must be from leader
                // only record incoming msg from channels other than ck
                if (nodeMember->getLeaderAddress().compare(s->getMarkerFormAddress()) != 0) {
                    Channel *c = s->getChannel(nodeMember->getLeaderAddress());
                    c->addMsg(recv_msg);
                }
                else {
                    Channel *c = s->getMarkerFromChannel();
                    c->addMsg(recv_msg);
                }
            }
            
            return "OK";
        } else if (strcmp(msg_type, D_LEAVEANNO) == 0) {
            // TODO: received: #LEAVEANNO#seq#name#ip:port sent by leader
            int param_seq = atoi(strtok(NULL, "#"));
            std::string param_name_addr(strtok(NULL, "#"));
            node->m_queue->push(std::make_pair(param_seq, "#"+std::string(msg_type)+"#"+param_name_addr));
            node->m_queue->pop();
            
            // snapshot
            if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
    std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                // msg must be from leader
                // only record incoming msg from channels other than ck
                if (nodeMember->getLeaderAddress().compare(s->getMarkerFormAddress()) != 0) {
                    Channel *c = s->getChannel(nodeMember->getLeaderAddress());
                    c->addMsg(recv_msg);
                }
                else {
                    Channel *c = s->getMarkerFromChannel();
                    c->addMsg(recv_msg);
                }
            }
            return "OK";
        }
        
    } else {
        
        if (strcmp(msg_type, D_CHAT) == 0) {

            // received: CHAT#ip:port#username::Message - From node to sequencer
            std::string addr(strtok (NULL, "#"));
            std::string chat_msg(strtok (NULL, "#"));
            if (nodeMember->isLeader()) { // Only leader can multicast messages
                // msg to be sent: #MSG#SEQ#addr#username::Message
                std::string message = chat_msg;
                node->multicastMsg(addr+"#"+message, D_M_MSG);
            }
            
            // snapshot
            if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
                std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                // only leader can receive this msg
                // only record incoming msg from channels other than ck
                if (addr.compare(s->getMarkerFormAddress()) != 0) {
                    Channel *c = s->getChannel(addr);
                    c->addMsg(recv_msg);
                }
                else {
                    Channel *c = s->getMarkerFromChannel();
                    c->addMsg(recv_msg);
                }
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
                // snapshot
                if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
    std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                    std::string addr(from_addr.getAddressIp()+":"+recv_port);
                    // only record incoming msg from channels other than ck
                    if (addr.compare(s->getMarkerFormAddress()) != 0) {
                        Channel *c = s->getChannel(addr);
                        c->addMsg(recv_msg);
                    }
                    else {
                        Channel *c = s->getMarkerFromChannel();
                        c->addMsg(recv_msg);
                    }
                }
                return message;
                
            } else {

                // received: JOINLEADER#LEADERIP#LEADERPORT
                // send back leader address - don't care failure, they can retry
                std::string recv_port(strtok (NULL, "#"));
                std::string recv_name(strtok (NULL, "#"));
                
                std::string message = std::string(D_JOINLEADER) + "#" + nodeMember->getLeaderAddressIp()
                + "#" + nodeMember->getLeaderAddressPort() + "#" + nodeMember->getLeaderName();
                
                // snapshot
                if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
    std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                    std::string addr(from_addr.getAddressIp()+":"+recv_port);
                    // only record incoming msg from channels other than ck
                    if (addr.compare(s->getMarkerFormAddress()) != 0) {
                        Channel *c = s->getChannel(addr);
                        c->addMsg(recv_msg);
                    }
                    else {
                        Channel *c = s->getMarkerFromChannel();
                        c->addMsg(recv_msg);
                    }
                }
                
                return message;

            }
        } else if (strcmp(msg_type, D_HEARTBEAT) == 0) {
            
            // received: HEARTBEAT#ip:port
            // know node at from_addr is still there, update heartbeat for node at from_addr
            std::string node_addr(strtok(NULL, "#"));
            time_t timev;
            time(&timev);
            nodeMember->updateHeartBeat(node_addr, timev);
//#ifdef DEBUGLOG
//            std::cout << "HeartBeat updated " << nodeMember->getHeartBeat(node_addr) << std::endl;
//#endif
            
            // snapshot
            if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
                std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                // only record incoming msg from channels other than ck
                if (node_addr.compare(s->getMarkerFormAddress()) != 0) {
                    Channel *c = s->getChannel(node_addr);
                    c->addMsg(recv_msg);
                }
                else {
                    Channel *c = s->getMarkerFromChannel();
                    c->addMsg(recv_msg);
                }
            }
            
            return "OK";

        } else if (strcmp(msg_type, D_ELECTION) == 0) {
            
            // TODO: received: ELECTION#ip:port
            std::string heardFrom(strtok(NULL, "#"));
            
            // If hears D_ELECTION from a process with a higher ID,
            if(heardFrom.compare(nodeMember->getAddress()) > 0) {

                std::cout << "\tElection: This should never happen " << heardFrom << std::endl;

             } else {
                 
                 std::cout << "\tElection: heard from someone smaller... " << heardFrom << std::endl;
                 // If hears D_ELECTION from a process with a lower ID, send back OK and startElection myself
                 // node->sendNotice(std::string(D_ANSWER) + "#" + nodeMember->getAddress(), heardFrom);
                 std::thread electionThread(&DNode::startElection, node);
                 electionThread.detach();
                 
            }
            
            // snapshot
            if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
    std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                // only record incoming msg from channels other than ck
                if (heardFrom.compare(s->getMarkerFormAddress()) != 0) {
                    Channel *c = s->getChannel(heardFrom);
                    c->addMsg(recv_msg);
                }
                else {
                    Channel *c = s->getMarkerFromChannel();
                    c->addMsg(recv_msg);
                }
            }

            return "OK";
            
        } else if (strcmp(msg_type, D_COOR) == 0) {
            
            std::cout << "\tReceive D_COOR " << std::endl;

            std::unique_lock<std::mutex> lk(node->mutex_election);

            // received: COOR#name#ip:port
            std::string leader_name(strtok(NULL, "#"));
            std::string heardFrom(strtok(NULL, "#"));
            
            node->updateElectionStatus(E_NONE);
            nodeMember->updateLeader(heardFrom, leader_name); // this one displays last leader left
            node->m_queue->resetSequence();
            
            std::cout << "\tD_COOR Done" << std::endl;
                        
            // snapshot
            if (node->getSnapshotStatus() == S_RECORDING) {
#ifdef DEBUGLOG
    std::cout << "Snapshot: in handling " << msg_type << std::endl;
#endif
                // only record incoming msg from channels other than ck
                if (heardFrom.compare(s->getMarkerFormAddress()) != 0) {
                    Channel *c = s->getChannel(heardFrom);
                    c->addMsg(recv_msg);
                }
                else {
                    Channel *c = s->getMarkerFromChannel();
                    c->addMsg(recv_msg);
                }
            }
            
            return "OK";
            
        } else if (strcmp(msg_type, D_MARKER) == 0) {
            // received: MARKER#ip:port
            std::string heardFrom(strtok(NULL, "#"));
            // if already received marker
            if (node->getSnapshotStatus() == S_RECORDING) {
                // record received marker
                s->recordChannelMarker(heardFrom);
                
                // mark state of cki as last time start recording cki
                Channel ck = *(s->getChannel(heardFrom));
                ck.copyChannelMsg(s->getMarkerFromChannel());
                
                // check whether finished snapshot
                if (s->receivedAllMarkers()) {  // finish snapshot
                    std::cout << "Snapshot complete! " << std::endl;
                    node->updateSnapshotStatus(S_NONE);
                }
            }
            // if have not seen marker
            else {
                Snapshot *new_s = node->startSnapshotByMarker(heardFrom);
                node->setSnapshot(new_s);
                
#ifdef DEBUGLOG
    std::cout << "After broadcast marker, channel num: " << new_s->getChannelNum() << std::endl;
#endif
                
                // check whether finished snapshot
                if (new_s->receivedAllMarkers()) {  // finish snapshot
                    std::cout << "Snapshot Complete!" << std::endl;
                    node->updateSnapshotStatus(S_NONE);
                }
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