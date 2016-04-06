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
#include <unordered_map>
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
#include <ifaddrs.h>
#include <netdb.h>

//using namespace std;

// You can comment out the following line
#define DEBUGLOG    1
#define MAXBUFLEN   512
#define TIMEOUT     10000
#define HEARTFREQ   3000

// Multicast messages, start with a #
#define D_M_MSG         "MSG"                      // #MSG#SEQ#Message, multicast from the sequencer
#define D_M_ADDNODE     "ADDNODE"                  // #ADDNODE#SEQ#ip#port#name, multicast from the sequencer

#define D_CHAT          "CHAT"                      // CHAT#Name#Message - From node to sequencer
#define D_JOINREQ       "JOINREQ"                   // JOINREQ#PORT
#define D_JOINLEADER    "JOINLEADER"                // JOINLEADER#LEADERIP#LEADERPORT
#define D_JOINLIST      "JOINLIST"                  // JOINLIST#initSeq#ip1:port1:name1:ip2:port2:name2...
#define D_LEAVE         "LEAVE"                     // LEAVE#name#ip:port
#define D_HEARTBEAT     "HEARTBEAT"                 // HEARTBEAT: () : detect failed node

#endif /* STDINCLUDES_H */