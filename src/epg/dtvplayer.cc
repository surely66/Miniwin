#include <ngl_types.h>
#include <ngl_log.h>
#include <ngl_os.h>
#include <ngl_msgq.h>
#include <ngl_video.h>
#include <si_table.h>
#include <dvbepg.h>
#include <ctype.h> 
#include <ngl_dmx.h>

NGL_MODULE(DTVPLAYER)

typedef struct{
    SERVICELOCATOR loc;
    char lan[3];
}MSGPLAY;

typedef struct {
   DTV_NOTIFY fun;
   void*userdata;
}DTVNOTIFY;

static DWORD msgQPlayer=0;
static SERVICELOCATOR sCurrentService;
static std::vector<DTVNOTIFY>gNotifies;

extern void DtvNotify(UINT,const SERVICELOCATOR*,DWORD wp,ULONG lp);
extern DWORD CreateFilter(USHORT pid,NGL_DMX_FilterNotify cbk,void*param,bool start,int masklen,...);

static INT PlayService(SERVICELOCATOR*sloc,const char*lan){
    USHORT pcr;
    INT vi=-1,ai=-1;
    ELEMENTSTREAM es[32];
    DtvTuneByService(sloc);    
    INT cnt=DtvGetServicePidInfo(sloc,es,&pcr);
    NGLOG_INFO("%d.%d.%d has %d elements lan=%s",sloc->netid,sloc->tsid,sloc->sid,cnt,lan);
    for(int i=0;i<cnt;i++){
       NGLOG_DEBUG("\t pid=%d type=%d lan=%s",es[i].pid,es[i].getType(),es[i].iso639lan);
       switch(es[i].getCategory()){
       case ST_VIDEO:vi=i;break;
       case ST_AUDIO:
           if((-1==ai)&&((0==lan[0])||(0==memcmp(es[i].iso639lan,lan,3)) ))
               ai=i;
           break;
       default:break;
       }
    }
    sCurrentService=*sloc;
    if(vi<0&&ai<0)return -1;
    else if(vi<0) nglAvPlay(0,0x1FFF,DECV_INVALID,es[ai].pid,es[ai].getType(),pcr);
    else if(ai<0) nglAvPlay(0,es[vi].pid,es[vi].getType(),0x1FFF,DECA_INVALID,pcr);
    else          nglAvPlay(0,es[vi].pid,es[vi].getType(),es[ai].pid,es[ai].getType(),pcr);
    DtvNotify(MSG_SERVICE_CHANGE,sloc,0,0);
}

static void SectionMonitor(DWORD filter,const BYTE *Buffer,UINT BufferLength, void *UserData){
     PMT p1((BYTE*)UserData,false);
     PMT p2(Buffer,false);
     if( /*(p1.tableId()==p2.tableId()) &&*/((p1.version()!=p2.version())||(p1.crc32()!=p2.crc32()))){
         DtvNotify((Buffer[0]==TBID_PMT?MSG_PMT_CHANGE:MSG_CAT_CHANGE),&sCurrentService,BufferLength,(ULONG)Buffer);
     }
     memcpy((BYTE*)UserData,Buffer,BufferLength);
}

static void PlayProc(void*param){
    MSGPLAY msg={{(USHORT)0,(USHORT)0,(USHORT)0xFFFF}};//0xFFFF  is an invalid serviceid
    SERVICELOCATOR lastplayed={0,0,(USHORT)0xFFFF};
    DWORD flt_pmt=0;
    DWORD flt_cat=0;
    BYTE PMT[1024];
    BYTE CAT[1024];
    int pmtpid;
    flt_cat=CreateFilter(PID_CAT,SectionMonitor,CAT,true,1,0xFF01);
    while(1){
        UINT count;
        int rc=NGL_ERROR;
        do{
            rc=nglMsgQReceive(msgQPlayer,&msg,sizeof(MSGPLAY),1000);
            nglMsgQGetCount(msgQPlayer,&count);
        }while( count && (rc==NGL_OK) );
        if(msg.loc.sid!=lastplayed.sid){
            lastplayed=msg.loc;
            DtvGetServicePmt(&lastplayed,PMT);
            DtvGetServiceItem(&lastplayed,SKI_PMTPID,&pmtpid);
            nglStopSectionFilter(flt_pmt);
            nglFreeSectionFilter(flt_pmt);
            flt_pmt=CreateFilter((USHORT)pmtpid,SectionMonitor,PMT,true,3,0xFF02,(0xFF00|(lastplayed.sid>>8)),(0xFF00|(lastplayed.sid&0xFF)));
            PlayService(&msg.loc,msg.lan);
        }
    }
}

static void Init(){
    if(0==msgQPlayer){
        DWORD thid;
        memset(&sCurrentService,0,sizeof(sCurrentService));
        msgQPlayer=nglMsgQCreate(10,sizeof(MSGPLAY));
        nglCreateThread(&thid,0,4096,PlayProc,NULL);
    }
}

void DtvGetCurrentService(SERVICELOCATOR*sloc){
    if(sloc)*sloc=sCurrentService;
}

INT DtvPlay(SERVICELOCATOR*loc,const char*lan){
    MSGPLAY msg;
    msg.loc=*loc;
    memset(msg.lan,0,3);
    if(lan)memcpy(msg.lan,lan,3);
    Init();
    nglMsgQSend(msgQPlayer,&msg,sizeof(MSGPLAY),100);
}

DWORD DtvRegisterNotify(DTV_NOTIFY notify,void*userdata){
     DTVNOTIFY n={notify,userdata};
     for(int i=0;i<gNotifies.size();i++){
          if(nullptr==gNotifies[i].fun){
               gNotifies[i].fun=notify;
               gNotifies[i].userdata=userdata;
               return i;
          }
     }
     gNotifies.push_back(n);
     NGLOG_VERBOSE("notify=%p",notify);
     return gNotifies.size()-1;
}

void DtvUnregisterNotify(DWORD hdl){
     if(hdl<gNotifies.size()){
          gNotifies[hdl].fun=nullptr;
          gNotifies[hdl].userdata=nullptr;
     }
}

void DtvNotify(UINT msg,const SERVICELOCATOR*svc,DWORD wp,ULONG lp){
     for(auto n=gNotifies.rbegin();n!=gNotifies.rend();n++){
          if((*n).fun!=nullptr)
            (*n).fun(msg,svc,wp,lp,(*n).userdata);
     }
}

