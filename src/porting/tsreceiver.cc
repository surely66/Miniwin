#include<tsreceiver.h>

int TSReceiver::InitSock(const char*ipaddr,int port){
    struct sockaddr_in addr;
#ifdef HAVE_IP_MREQN
    struct ip_mreqn mgroup;
#else
    struct ip_mreq mgroup;
#endif
    int addrlen,reuse;
    memset((char *) &mgroup, 0, sizeof(mgroup));
    mgroup.imr_multiaddr.s_addr = inet_addr(ipaddr);
#ifdef HAVE_IP_MREQN
    mgroup.imr_address.s_addr = INADDR_ANY;
#else
    mgroup.imr_interface.s_addr = INADDR_ANY;
#endif
    memset((char *) &addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ipaddr);
    addrlen = sizeof(addr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    reuse = 1;
    printf("ip:%s port:%d\r\n",ipaddr,port);
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse))<0){
        printf("setsockopt() SO_REUSEADDR: error\r\n");
    }
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
	printf("bind(): error\r\n");
	close(sockfd);
	return -1;
    }
    return sockfd;  
}
int TSReceiver::StartReceive(){
    char tsbuff[TSPACK_SIZE];
    while(1){
        int len=recvfrom(sockfd, tsbuff,TSPACK_SIZE, 0,NULL,NULL);// (struct sockaddr *) &addr,&addrlen)
        if(CallBack&&len>0)CallBack(tsbuff,len,userdata);        
    }
}

#ifdef BUILD_RCV_DEMO
static void TSCBK(void*pdata,int len,void*userdata){
    uint8_t*data=(uint8_t*)pdata;
    printf("%02x %02x %02x %02x",data[0],data[1],data[2],data[3]);
}
int main(int argc,char*argv[]){
    TSReceiver rcv;
    if(argc<3){
        printf("Usage:%s ipaddr port\r\n",argv[0]);
        return -1;
    }
    rcv.InitSock(argv[1],atoi(argv[2]));
    rcv.SetCallback(TSCBK,NULL);
    rcv.StartReceive();
    return 0;
}
#endif
