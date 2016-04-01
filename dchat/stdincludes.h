/**********************************
 * FILE NAME: stdincludes.h
 *
 * DESCRIPTION: standard header file
 **********************************/

#ifndef STDINCLUDES_H
#define STDINCLUDES_H

/*
 * Macros
 */
#define FAILURE -1
#define SUCCESS 0

/*
 * Standard Header files
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <execinfo.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <queue>
#include <deque>
#include <list>
#include <fstream>
#include <thread>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

//using namespace std;

// You can comment out the following line
#define DEBUGLOG 1

#define MAXBUFLEN 512

#define D_CHAT "CHAT"                   // DCHAT:Name:Message - From node to sequencer
#define D_MSG "MSG"                     // MSG:SEQ#:Message - From sequence to node
#define D_JOINREQ "JOINREQ"             // JOINREQ:PORT
#define D_JOINLEADER "JOINLEADER"       // JOINOK:ip:port
#define D_JOINLIST "JOINLIST"           // JOINOK:ip1:port1:ip2:port2:...
#endif /* STDINCLUDES_H */
