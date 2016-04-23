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

#define MAXBUFLEN       512
#define TIMEOUT         3000
#define HEARTFREQ       2000
#define ELECTIONTIME    4000

#define D_PROPOSE       "PROPOSE"                   // PROPOSE#MSG
#define D_COMMIT        "COMMIT"                    // COMMIT#id#pid

#define D_JOINREQ       "JOINREQ"                   // JOINREQ#PORT#Name
#define D_JOINLIST      "JOINLIST"                  // JOINLIST#LEADERNAME#ip1:port1:name1:...

#define D_M_CHAT        "CHAT"                      // CHAT#seq#msg
#define D_M_ADDNODE     "ADDNODE"                   // ADDNODE#seq#ip#port#name, multicast from the sequencer

#define D_HEARTBEAT     "HEARTBEAT"                 // HEARTBEAT#ip:port detect failed node


#endif /* STDINCLUDES_H */