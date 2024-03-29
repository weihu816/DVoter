/**********************************
 * FILE NAME: DNet.cpp
 *
 * DESCRIPTION: Network classes definition
 **********************************/

#include "DNet.h"

DNet::DNet(Handler * handler) {
    this->handler = handler;
}

/**
 * FUNCTION NAME: ENinit
 *
 * DESCRIPTION: Init the emulnet for this node
 *              Randomly choose port number
 */
int DNet::DNinit() {
    
    // randomly choose port number
    srand((unsigned int) time(NULL));
    port = rand() % 500 + 20000;
    
    struct addrinfo hints, *servinfo, *p;
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // AF_UNSPEC: set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;     // UDP datagram
    hints.ai_flags = AI_PASSIVE;        // use my IP
    
    // NULL means
    if ((rv = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return FAILURE;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
#ifdef DEBUGLOG
            perror("socket");
#endif
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
#ifdef DEBUGLOG
            perror("bind");
#endif
            continue;
        }
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return FAILURE;
    }
    
    freeaddrinfo(servinfo);
    
#ifdef DEBUGLOG
    std::cout << "DEBUG:(DNet::DNinit) return " << SUCCESS << std::endl;
#endif
    
    return SUCCESS;
}

/**
 * FUNCTION NAME: get_in_addr
 *
 * DESCRIPTION: get sockaddr, IPv4 or IPv6
 */
void * DNet::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    } else {
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

/**
 * FUNCTION NAME: get_in_addr
 *
 * DESCRIPTION: get port number, IPv4 or IPv6
 *
 * RETURNS: port number
 *
 */
in_port_t get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return (((struct sockaddr_in*)sa)->sin_port);
    } else {
        return (((struct sockaddr_in6*)sa)->sin6_port);
    }
}


/**
 * FUNCTION NAME: DNsend
 *
 * DESCRIPTION: Message send successfully or not
 *              Receive ACK using at least once semantic
 */
int DNet::DNsend(Address * addr, std::string data, std::string & ack, int times) {
#ifdef DEBUGLOG
    std::cout << "\tDNet::DNsend: " << addr->getAddress() << " " << data << std::endl;
#endif
    if (times <= 0) return FAILURE;
    
    int sockfd_w = 0;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    long numbytes;
    
    // set timeout val
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    // receive msg from server, for ack
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXBUFLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_UNSPEC/AF_INET
    hints.ai_socktype = SOCK_DGRAM;
    
    const char * addr_ip = addr->ip.c_str();
    const char * addr_port = std::to_string(addr->port).c_str();
    if ((rv = getaddrinfo(addr_ip, addr_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return FAILURE;
    }
    
    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_w = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
#ifdef DEBUGLOG
            perror("stub: socket error");
#endif
            continue;
        }
        if (setsockopt(sockfd_w, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(struct timeval)) < 0) {
#ifdef DEBUGLOG
            perror("setsockopt");
#endif
            return FAILURE;
        }
        break;
    }
    
    if (p == NULL) {
#ifdef DEBUGLOG
        fprintf(stderr, "stub: failed to create socket.\n");
#endif
        close(sockfd_w);
        return FAILURE;
    }
    
    // Send message
    strcpy(buf, data.c_str());
    if ((numbytes = sendto(sockfd_w, buf, strlen(buf) + 1, 0, p->ai_addr, p->ai_addrlen)) == -1) {
#ifdef DEBUGLOG
        perror("sendto");
#endif
        close(sockfd_w);
        return FAILURE;
    }
    
    // To receive ACK from the server
    if ((numbytes = recvfrom(sockfd_w, buf, MAXBUFLEN - 1, 0,
                             (struct sockaddr *) &their_addr, &addr_len)) == -1) {
#ifdef DEBUGLOG
        perror("recvfrom");
#endif
        // not getting ack within timeout
        close(sockfd_w);
        return DNsend(addr, data, ack, times - 1);
    }
    
#ifdef DEBUGLOG
    std::cout << "\tDNet::DNsend: receive: " << data << std::endl;
#endif
    
    ack = std::string(buf, numbytes);
    close(sockfd_w);
    freeaddrinfo(servinfo);
    return SUCCESS;
}

/**
 * FUNCTION NAME: DNrecv
 *
 * DESCRIPTION: Receives msg and returns SUCCESS or FAILURE
 *              If timeout < 0, no timeout
 */
int DNet::DNrecv(Address &fromaddr, std::string &data) {
    
    long numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXBUFLEN];
    char s[INET6_ADDRSTRLEN];
    
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1 , 0,
                             (struct sockaddr *) &their_addr, &addr_len)) == -1) {
#ifdef DEBUGLOG
        perror("recvfrom");
#endif
        return FAILURE;
    }
    
    // copy their_addr to fromaddr
    fromaddr.ip = std::string(inet_ntop(their_addr.ss_family,
                                        get_in_addr((struct sockaddr *) &their_addr), s, sizeof s));
    fromaddr.port = ntohs(get_in_port((struct sockaddr *) &their_addr));
    data = std::string(buf, numbytes);
    
#ifdef DEBUGLOG
    std::cout << "\treceive: " << data << " from "
    << inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s)
    << ":" << ntohs(get_in_port((struct sockaddr *) &their_addr))
    << std::endl;
#endif
    
    std::string recv_msg(buf);
    
    // get message type and port from message
    char cstr[MAXBUFLEN];
    strcpy(cstr, recv_msg.c_str());
    char * msg_type = strtok(cstr, "#");
    std::string recv_port(strtok (NULL, "#"));
    //std::cout << "\tMsg type: " << msg_type << std::endl;
    
    if(handler->amLeader() && recv_msg.find("#") != 0 && strcmp(msg_type, D_JOINREQ) != 0 && strcmp(msg_type, D_HEARTBEAT) != 0) { // am leader, not broadcast, not join request, not heartbeat..
        //std::cout << "\tI am the leader, put things in queue...\n" << std::endl;
        // process in another way... first put in queue
        std::string from_ip_port = fromaddr.ip +":"+ recv_port;
        //std::cout << "\tRecv from: " <<  from_ip_port << std::endl;
        leader_round_table[from_ip_port].push_back(socket_queue_item(sockfd, &their_addr, fromaddr, recv_msg));
        processByRoundTable();
        return SUCCESS;
    } else {
        
        // if I am not leader: Handle received message
        std::string send_msg = handler->process(fromaddr, recv_msg);
        
#ifdef DEBUGLOG
        printf("\tsend back: %s\n", send_msg.c_str());
#endif
        
        // Fail to handle message, simply no sending back, let the send timeout
        if (send_msg.empty()) return FAILURE;
        
        // Send ack back
        strcpy(buf, send_msg.c_str());
        
        if ((numbytes = sendto(sockfd, buf, strlen(buf) + 1, 0,
                               (struct sockaddr *) &their_addr, addr_len)) == -1) {
            perror("sendto");
            return FAILURE;
        }
        
        return SUCCESS;
    }
}

int DNet::processByRoundTable() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // sleep for 0.5 sec
    
    char buf[MAXBUFLEN];
    long numbytes;
    int ite;
    std::string targetKey;
    // pop one from the queue (round-robin) and process
    int memberListLength = handler->getMemberListLength();
    
    socket_queue_item poped;
    for (ite = 0; ite <= memberListLength; ite++) {
        //printf("try POP...The current member List Counter is %d\n", tableCounter);
        targetKey = handler->getMemberAddrByIndex(tableCounter);
        //std::cout << "target key is : " << targetKey << std::endl;
        if(leader_round_table[targetKey].tryPop(poped)) {
            //std::cout << "found msg in queue, processing.. " << std::endl;

            if(targetKey.compare(handler->getAddress()) == 0) {
                // I may receive my own chat messages
                handler->multicast(poped.message, D_M_MSG);
                tableCounter++;
                return SUCCESS;
            }
            
            tableCounter++;
            std::string send_msg = handler->process(poped.from_addr, poped.message);
            if(send_msg.empty()) return FAILURE;
            strcpy(buf, send_msg.c_str());
            
            if ((numbytes = sendto(poped.sockfd, buf, strlen(buf) + 1, 0,
                                   (struct sockaddr *) &poped.their_addr, sizeof(poped.their_addr))) == -1) {
                perror("sendto");
                return FAILURE;
            }
            return SUCCESS;
        }
        tableCounter++;
    }
    return FAILURE;
    
    
}

/**
 * FUNCTION NAME: DNinfo
 *
 * DESCRIPTION: Get local ip and port information
 *              Returns SUCCESS if it succeeds, FAILURE if fails
 */
int DNet::DNinfo(std::string & addr) {
    std::string str_ip, str_port;
    struct ifaddrs * ifaddr, * ifa;
    char host[NI_MAXHOST];
    
    if (getifaddrs(&ifaddr) == -1) {
#ifdef DEBUGLOG
        perror("getifaddrs");
#endif
        return FAILURE;
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        // mac:en0, linuxs:eth0, unix:em1
        if ((strcmp(ifa->ifa_name, "em1") == 0) && (ifa->ifa_addr->sa_family == AF_INET)) {
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                                NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                return FAILURE;
            }
            str_ip = std::string(host);
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    
    struct sockaddr_in local_address;
    socklen_t address_length = sizeof local_address;
    getsockname(sockfd, (struct sockaddr*) &local_address, &address_length);
    
    str_port = std::to_string((int) ntohs(local_address.sin_port));
    addr = str_ip + ":" + str_port;
    
#ifdef DEBUGLOG
    std::cout << "DEBUG:(DNet::DNinfo) return " << SUCCESS << std::endl;
#endif
    
    return SUCCESS;
}


