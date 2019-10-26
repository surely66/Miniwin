#ifndef __TSSENDER_H__
#define __TSSENDER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#ifndef TSPACK_SIZE
#define TSPACK_SIZE (7*188)
#endif

class TSSender{
private:
    struct sockaddr_in addr;
    int64_t Diff(struct timeval*,struct timeval*);
protected:
    int sockfd;
    int completed;
public:
    TSSender(){
       sockfd=-1;
       completed=1;
    }
    int InitSock (const char*ip,int port);
    int StartSend(const char*fname,uint32_t bitrate=38000000);
    int Stop(){completed=1;}
};
#endif

