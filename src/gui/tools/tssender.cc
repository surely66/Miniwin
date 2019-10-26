#include<tssender.h>
class TSReceiver{
protected:
    int sockfd;
    int InitSock(const char*ip,int port);    
};

int TSSender::InitSock(const char*ipaddr,int port){
    int len;
    unsigned int bitrate;
    unsigned long long int packet_time;
    unsigned long long int real_time;    
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipaddr);
    addr.sin_port = htons(port);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("TSSender::InitSock ip:%s port:%d\r\n",ipaddr,port);
    return sockfd;
}

int64_t TSSender::Diff(struct timeval* time_stop, struct timeval* time_start)
{
    int64_t temp = 0;
    int64_t utemp = 0;
		   
    if (time_stop && time_start) {
	if (time_stop->tv_usec >= time_start->tv_usec) {
		utemp = time_stop->tv_usec - time_start->tv_usec;    
		temp = time_stop->tv_sec - time_start->tv_sec;
	} else {
		utemp = time_stop->tv_usec + 1000000 - time_start->tv_usec;       
		temp = time_stop->tv_sec - 1 - time_start->tv_sec;
	}
	if (temp >= 0 && utemp >= 0) {
		temp = (temp * 1000000) + utemp;
       	} else {
		printf("start time %ld.%ld is after stop time %ld.%ld\n", time_start->tv_sec, time_start->tv_usec, time_stop->tv_sec, time_stop->tv_usec);
		temp = -1;
	}
    } else {
	printf("memory is garbaged?\n");
	temp = -1;
    }
    return temp;
}

int TSSender::StartSend(const char*tsfile,uint32_t bitrate){
    int nloop=0;
    char tsbuff[TSPACK_SIZE];
    uint64_t packet_time=0;
    uint64_t real_time; 
    struct timeval time_start;
    struct timeval time_stop;
    struct timespec nano_sleep_packet;
    int fd;
    memset(&nano_sleep_packet, 0, sizeof(nano_sleep_packet));
    fd=open(tsfile, O_RDONLY);
    gettimeofday(&time_start, 0);
    printf("send file:%s fd=%d socket=%d\r\n",tsfile,fd,sockfd);
    completed=0;
    while(!completed){
        gettimeofday(&time_stop, 0);
        real_time =Diff(&time_stop, &time_start);
        if (real_time * bitrate > packet_time * 1000000){
            int len = read(fd, tsbuff, sizeof(tsbuff)); 
            if(len==0){//send complete
               lseek(fd,0,SEEK_SET);
            }else if(len<0){
            }else{
                int sent = sendto(sockfd,tsbuff,len, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
                packet_time += len * 8;
            } 
        }else{
            nanosleep(&nano_sleep_packet, 0);
        }
    }
    close(fd);
}

#ifdef BUILD_SND_DEMO
int main(int argc,char *argv[]){
    TSSender snd;
    uint32_t bitrate=38000000;
    if(argc<4){
       printf("Usage:%s tsfile ipaddr port [bitrate]\r\n",argv[0]);
       return -1;
    }
    snd.InitSock(argv[2],atoi(argv[3]));
    snd.StartSend(argv[1],bitrate);
}
#endif
