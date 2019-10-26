#ifndef __TSRECEIVER_H__
#define __TSRECEIVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#ifndef TSPACK_SIZE
#define TSPACK_SIZE (7*188)
#endif
typedef void(*WRITE_TS_CALLBACK)(void*buf,int len,void*userdata);
class TSReceiver{
protected:
    int sockfd;
    WRITE_TS_CALLBACK CallBack;
    void*userdata;
public:
    TSReceiver(){
        sockfd=-1;
        CallBack=NULL;
    }
    int InitSock(const char*ip,int port);
    int StartReceive();
    int SetCallback(WRITE_TS_CALLBACK cbk,void*usrdata){
        CallBack=cbk;
        userdata=usrdata;
    }
};
#endif
