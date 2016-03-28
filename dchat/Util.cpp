/**********************************
 * FILE NAME: Util.cpp
 *
 * DESCRIPTION: 
 **********************************/

#include "stdincludes.h"

static int sockfd; // listering socket

/**
 * FUNCTION NAME: create_addr
 *
 * DESCRIPTION: return ADDR:PORT; NULL if it fails
 */
std::string create() {
    
    std::string port = "8080";
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP datagram
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
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

std::string receive() {
    
    struct sockaddr_storage their_addr; // connector's address information
    size_t numbytes;
    char buf[MAXBUFLEN];
    
    socklen_t addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1 , 0,
                             (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    
    // Handle the message
    
    
    return NULL;
}

