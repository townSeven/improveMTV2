#ifndef _SOCKETCONN_H
#define _SOCKETCONN_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
//#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/un.h>
#include <pthread.h>
//#include <arpa/inet.h>


void* runServer (int* sockfd, int* newfd);

//void  sendResultToClient(int* fd, ddtacontext* pContext);

#endif
