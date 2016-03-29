/**********************************
 * FILE NAME: DNet.cpp
 *
 * DESCRIPTION: Network classes definition
 **********************************/

#include "DNet.h"


/**
 * FUNCTION NAME: ENinit
 *
 * DESCRIPTION: Init the emulnet for this node
 *              Randomly choose port number
 */
std::string DNet::DNinit() {
    // Randomly choose port number
    srand((unsigned int) time(NULL));
    int port = rand() % 500 + 20000;
    
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP datagram
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return NULL;
    }

    // TODO Only for IPv4
    struct sockaddr_in *sa = (struct sockaddr_in *) servinfo->ai_addr;
    char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
    inet_ntop(AF_INET, &(sa->sin_addr), ip4, INET_ADDRSTRLEN);
    int port_ = ntohs(sa->sin_port);

#ifdef DEBUGLOG
    printf("IP=%s\n", ip4);
    printf("Port=%d\n", port_);
#endif

    std::string address(ip4);
    address += ":" + std::to_string(port_);
    freeaddrinfo(servinfo);
    return address;
}

/**
 * FUNCTION NAME: ENsend
 *
 * DESCRIPTION: EmulNet send function
 *
 * RETURNS: ???
 */
int DNet::DNsend(Address * addr, std::string data) {
#ifdef DEBUGLOG
    std::cout << "DNsend: " << addr->getAddress() << std::endl;
#endif
    int sockfd_w = 0;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    
    // receive msg from server, for ack
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    
    char* fullmsg = new char[MAXBUFLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(addr->ip.c_str(), std::to_string(addr->port).c_str(),
                          &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return FAILURE;
    }
    
    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_w = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("stub: socket error");
            continue;
        }
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (setsockopt(sockfd_w, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(struct timeval)) == -1) {
            perror("setsockopt");
            return FAILURE;
        }
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "stub: failed to get addr.\n");
        return FAILURE;
    }
    
    // TODO: assign sequence number and add to mes queue
    

    // Free resources
    close(sockfd_w);
    
    return SUCCESS;
}

int DNet::DNrecv(int (*enq)(void *, char *, int), struct timeval *t, int times, void *queue) {
    
    return SUCCESS;
}



