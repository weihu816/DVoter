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
    port = rand() % 500 + 20000;
    
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
 * FUNCTION NAME: get_in_addr
 *
 * DESCRIPTION: get sockaddr, IPv4 or IPv6
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
    }
    
    return (((struct sockaddr_in6*)sa)->sin6_port);
}

/**
 * FUNCTION NAME: DNsend
 *
 * DESCRIPTION: send msg
 *
 * RETURNS: Message send successfully or not
 */
int DNet::DNsend(Address * addr, std::string data) {
#ifdef DEBUGLOG
    std::cout << "DNsend: " << addr->getAddress() << std::endl;
#endif
    int sockfd_w = 0;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    size_t numbytes;
    
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
        fprintf(stderr, "stub: failed to create socket.\n");
        return FAILURE;
    }
    
    if ((numbytes = sendto(sockfd_w, data.c_str(), strlen(data.c_str()), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("sendto");
        exit(1);
    }

#ifdef DEBUGLOG
    std::cout << "Send " << numbytes << " bytes to " << addr->getAddress() << std::endl;
#endif
    // Free resources
    freeaddrinfo(servinfo);
    close(sockfd_w);
    return SUCCESS;
}

/**
 * FUNCTION NAME: DNrecv
 *
 * DESCRIPTION: receive msg
 *
 * RETURNS: Message received successfully or not
 */
int DNet::DNrecv(Address & fromaddr, std::string & data) {
    int sockfd_r = 0;
    size_t numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return FAILURE;
    }
    
    // loop through all the results and make a scocket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_r = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(struct timeval)) == -1) {
            perror("setsockopt");
            return FAILURE;
        }
        if (bind(sockfd_r, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd_r);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    
    freeaddrinfo(servinfo);
    
    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd_r, buf, MAXBUFLEN-1 , 0,
                             (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    
    //copy their_addr to fromaddr
    fromaddr.ip = inet_ntop(their_addr.ss_family,
                            get_in_addr((struct sockaddr *)&their_addr),
                            s, sizeof s);
    fromaddr.port = ntohs(get_in_port((struct sockaddr *)&their_addr));

    buf[numbytes] = '\0';
#ifdef DEBUGLOG
    printf("listener: got packet from %s\n",
           inet_ntop(their_addr.ss_family,
                     get_in_addr((struct sockaddr *)&their_addr),
                     s, sizeof s));
    printf("listener: packet is %d bytes long\n", (int)numbytes);
    printf("listener: packet contains \"%s\"\n", buf);
#endif
    
    data = std::string(buf);
    close(sockfd_r);
    
    return SUCCESS;
}



