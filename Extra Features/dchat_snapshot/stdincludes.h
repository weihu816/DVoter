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
#include <mutex>
#include <chrono>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <netdb.h>

//using namespace std;

// You can comment out the following line
//#define DEBUGLOG    1
//#define CIPHERLOG   1
//#define CHANNELDEMO   1

#define MAXBUFLEN       512
#define HEARTFREQ       3000
#define COORTIMEOUT     8000
#define ENCRYPTKEY      13

// Multicast messages, start with a #
#define D_M_MSG         "MSG"               // #MSG#SEQ#addr#username::Message, multicast from the sequencer
#define D_M_ADDNODE     "ADDNODE"           // #ADDNODE#SEQ#ip#port#name, multicast from the sequencer
#define D_LEAVEANNO     "LEAVEANNO"         // #LEAVEANNO#seq#name#ip:port

#define D_CHAT          "CHAT"              // CHAT#port#Seq#username::Message - From node to sequencer
#define D_JOINREQ       "JOINREQ"           // JOINREQ#PORT#Name
#define D_JOINLEADER    "JOINLEADER"        // JOINLEADER#LEADERIP#LEADERPORT#LEADERNAME
#define D_JOINLIST      "JOINLIST"          // JOINLIST#initSeq#LEADERNAME#ip1:port1:name1:...
#define D_HEARTBEAT     "HEARTBEAT"         // HEARTBEAT#ip:port detect failed node
#define D_ELECTION      "ELECTION"          // ELECTION#ip:port, start election
#define D_COOR          "COOR"              // COOR#name#ip:port, announce leadership
#define D_MARKER        "MARKER"            // MARKER#ip:port, multicast to all others


// Election status
#define E_NONE  0       // not in election
#define E_WAITANS   1   // wait for higher process's D_ANSWER
#define E_WAITCOOR  2   // wait for higher process's D_COOR

// Snapshot status
#define S_NONE 0        // not in taking snapshot
#define S_RECORDING 1   // first time see marker, start recording

#endif /* STDINCLUDES_H */