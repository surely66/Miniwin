#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<tssender.h>
#include<tsreceiver.h>
#include<pthread.h>
#include<map>
#include<string>
static TSSender snd;
static TSReceiver rcv;
static uint32_t bitrate=38000000;
static std::map<int,std::string>freqs;
static std::string broadcast_ip="127.0.0.1";
static std::string command_ip="127.0.0.1";
static int command_port=18800;
static int broadcast_port=18880;

static void*SendProc(void*p){
    int freq=(int)(long)p; 
    std::string fname=freqs[freq];
    snd.StartSend(fname.c_str(),bitrate);
}

static void NIM_CMD_CBK(void*data,int len,void*userdata){
    int freq=atoi((char*)data);
    pthread_t tid;
    snd.Stop();
    usleep(10000); 
    pthread_create(&tid,NULL,SendProc,(void*)freq);
}

static void*NimProc(void*p){
   rcv.SetCallback(NIM_CMD_CBK,NULL);
   rcv.StartReceive(); 
}

static int read_config(const char*fname){
    FILE*f=fopen(fname,"r");
    int len,lineno=0;
    char*line=NULL,*p;
    
    size_t alloc_len;
    while((len=getline(&line,&alloc_len,f))!=-1){
        char*token;
        if(line[len-1]=='\n')line[len-1]=0;
        p=line;
        lineno++;
        while(isspace(*p))p++;
        if('#'==*p||'\0'==*p)continue;
        token=strtok(p,"=");
        if(NULL==token)continue;
        if(isdigit(*token)){
            char*tsfile=strtok(NULL,"=");
            struct stat st;
            if((stat(tsfile,&st)==0)&&S_ISREG(st.st_mode)){
                freqs[atoi(token)]=tsfile;
                printf("%s=%s\r\n",token,tsfile);
            }else{
                printf("tsfile %s not exists at line %d",tsfile,lineno);
            }
        }else{
            char*param=strtok(NULL,"=");
            if(strcmp(token,"COMMAND_PORT")==0)
                command_port=atoi(param);
            else if(strcmp(token,"COMMAND_IP")==0)
                command_ip=param;
            else if(strcmp(token,"BROADCAST_PORT")==0)
                broadcast_port=atoi(param);
            else if(strcmp(token,"BROADCAST_IP")==0)
                broadcast_ip=param;
        }
    }
    free(line);
    fclose(f);
}
int main(int argc,char *argv[]){
    pthread_t tid;
    if(argc<2){
       printf("Usage:%s config.ini",argv[0]);
       return -1;
    }
    read_config(argv[1]);
    snd.InitSock(broadcast_ip.c_str(),broadcast_port);
    rcv.InitSock(command_ip.c_str(),command_port);
    pthread_create(&tid,NULL,NimProc,&snd);
    NIM_CMD_CBK((void*)"10750",6,NULL);
    pthread_join(tid,NULL);
}

