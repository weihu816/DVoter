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
#include <fstream>
#include <thread>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

//using namespace std;

//#define STDCLLBKARGS (void *env, char *data, int size)
//#define STDCLLBKRET	void

// You can comment out the following line
#define DEBUGLOG 1

#define MAXBUFLEN 512

#define D_CHAT "CHAT" // DCHAT:Name:Message - From node to sequencer
#define D_JOIN "JOIN"
#define D_MSG "MSG"
#endif /* STDINCLUDES_H */
