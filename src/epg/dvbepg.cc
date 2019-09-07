//#include<program_manager.h>
#include <list>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <ngl_types.h>
#include <ngl_os.h>
#include <ngl_dmx.h>
#include <ngl_msgq.h>
#include <ngl_video.h>
#include <ngl_tuner.h>
#include <ngl_log.h>
#include <dvbepg.h>
#include <tvdata.h>
#include <favorite.h>
#include <lrucache.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <utils.h>
#include <sys/time.h>
#include <satellite.h>

NGL_MODULE(EPG)

using namespace std;

enum{
   MSG_IDLE,
   MSG_START_SEARCH,
   MSG_TUNNING,//tunning request
   MSG_LOCK,   //tunning result
   MSG_SEARCH_TP,//begin search transport stream 
   MSG_PAT_RECEIVED,//pat received
   MSG_PMT_RECEIVED,
   MSG_SDT_RECEIVED,
   MSG_NIT_RECEIVED,
   MSG_BAT_RECEIVED,
   MSG_FINISHED_TP,
   MSG_SEARCH_FINISHED,//seatch
};
enum{
   STATE_IDLE,
   STATE_STARTED,
   STATE_TUNNING,
   STATE_PSI_GETTING,
   STATE_PSI_GETTED,
};
const char*MSGNAME[]={  "MSG_IDLE","MSG_START_SEARCH","MSG_TUNNING","MSG_LOCK","MSG_SEARCH_TP", "MSG_PAT_RECEIVED","MSG_PMT_RECEIVED",
                        "MSG_SDT_RECEIVED","MSG_NIT_RECEIVED","MSG_FINISHED_TP","MSG_SEARCH_FINISHED","YOUR MSG"  };

typedef struct{
    DWORD msgid;
    DWORD param1;
    DWORD param2;
}EPGMSG;

static std::vector<NGLTunerParam>schFrequencies;
static DWORD fltEIT, fltTDT, fltBAT;
static DWORD msgQ=0;
static void NotifyServiceArrived(STREAMDB&ts,SEARCHNOTIFY*notify);
extern void DtvNotify(UINT,const SERVICELOCATOR*);
static void SENDMSG(DWORD id,DWORD p1,DWORD p2){
    EPGMSG msg={id,p1,p2};
    nglMsgQSend(msgQ,&msg,sizeof(EPGMSG),100);
}

static void SectionCBK(DWORD filter,const BYTE *Buffer,UINT BufferLength, void *UserData){
    PSITable tbl(Buffer);
    STREAMDB*ts=(STREAMDB*)UserData;
    SECTIONLIST::iterator itr;
    int notfound,changed;
    switch(tbl.tableId()){
    case TBID_PAT://PAT;
         ts->tsid=tbl.extTableId();
         itr=std::find(ts->pat.begin(),ts->pat.end(),tbl);
         if(notfound=(itr==ts->pat.end()))
              ts->pat.push_back(tbl);
         NGLOG_VERBOSE("tbid=%02x exttbid=%x section cnt=%d",tbl.tableId(),tbl.extTableId(),ts->pat.size());
         SENDMSG(MSG_PAT_RECEIVED,tbl.extTableId(),filter);
         break;
    case TBID_CAT:break;//CAT;
    case TBID_PMT://PMT;
         itr=std::find(ts->pmt.begin(),ts->pmt.end(),tbl);
         if(notfound=(itr==ts->pmt.end()))
             ts->pmt.push_back(tbl);
         //NGLOG_VERBOSE_IF(notfound,"\t*serviceid=%d section cnt=%d filt=%x pid=%d found=%d",psitbl.extTableId(),
         //     ts->pmt.size(),filter, nglGetFilterPid(filter),!notfound);
         if(notfound)SENDMSG(MSG_PMT_RECEIVED,(ts->pmt.size()<<16|tbl.extTableId()),filter);//program_number
         break;
    case TBID_SDT://SDT
    case TBID_SDT_OTHER://SDT
         itr=find(ts->sdt.begin(),ts->sdt.end(),tbl);
         if(notfound=(itr==ts->sdt.end()))
              ts->sdt.push_back(tbl);
         else *itr=tbl;
         if(notfound)SENDMSG(MSG_SDT_RECEIVED,tbl.extTableId(),filter);//streamid
         break;
    case TBID_BAT:
         AddBATSection((BAT&)tbl,&changed);
         break;
    case TBID_NIT:
    case TBID_NIT_OTHER:{NGLOG_DEBUG("===RCV NIT===");
             SECTIONLIST*nitsecs=(SECTIONLIST*)UserData;
             itr=std::find(nitsecs->begin(),nitsecs->end(),tbl);
             if(itr==nitsecs->end())nitsecs->push_back(tbl);
                 SENDMSG(MSG_NIT_RECEIVED,0,filter);
         }break;
    case TBID_EITPF:
    case TBID_EITPF_OTHER:
         {
             EIT eit(tbl,false);
             SERVICELOCATOR loc=eit;
             AddEITPFSection(eit,&changed);
             if(changed&&(eit.tableId()==TBID_EITPF)){
                  //NGLOG_DEBUG("%d.%d.%d pf update",loc.netid,loc.tsid,loc.sid);
                  DtvNotify(MSG_EPG_PF,&loc);
             }
         }break;
    case TBID_EITS...TBID_EITS_LAST:
    case TBID_EITS_OTHER...TBID_EITS_OTHER_LAST:
         {
             EIT eit(tbl,false);
             SERVICELOCATOR loc=eit;
             AddEITSSection(eit,&changed);
             if(changed && ((eit.tableId()&0x50)==0x50))DtvNotify(MSG_EPG_SCHEDULE,&loc);
         }break;
    case TBID_TDT:
    case TBID_TOT:{
             TOT tot(tbl);
             NGL_TIME tc;
             NGL_TM ts;
             INT mjd,utc;
             tot.getTime(mjd,utc);
             UTC2Tm(mjd,utc,&ts);
             UTC2Time(mjd,utc,&tc);
             int rc=nglSetTime(&tc); 
             if(tot.tableId()==TBID_TOT){
                 TimeOffset tms[32];
                 int cnt=tot.getTimeOffset(tms,32);
                 for(int i=0;i<cnt;i++){
                      NGLOG_VERBOSE("%d:country:%s timeoffset=%d polarity:%d",i,tms[i].country_code,tms[i].local_time_offset,tms[i].polarity);
                 }
             }
             NGLOG_DEBUG_IF(ts.uiSec%30==0,"Recv %s Time:%d/%d/%d %02d:%02d:%02d",(tot.tableId()==TBID_TDT?"TDT":"TOT"),
                 ts.uiYear,ts.uiMonth+1,ts.uiMonthDay,ts.uiHour,ts.uiMin,ts.uiSec);   
         }break;
    default:NGLOG_DEBUG("todo tbid:%x",tbl.tableId());break;         
    }
}

static DWORD CreateFilter(USHORT pid,BYTE*mask,BYTE*match,int masklen,void*param,bool start=true){
    DWORD flt=nglAllocateSectionFilter(0,pid,SectionCBK,param,NGL_DMX_SECTION);
    nglSetSectionFilterParameters(flt,mask,match,masklen);
    if(start)nglStartSectionFilter(flt);
    return flt;
}

static void TunningCBK(INT tuneridx,INT lockedState,void*param){
    EPGMSG msg;
    msg.msgid=MSG_LOCK;
    msg.param1=lockedState;
    msg.param2=0;
    nglMsgQSend(msgQ,&msg,sizeof(EPGMSG),1500);
    NGLOG_DEBUG("locked=%d",lockedState);
}

static void  SearchProc(void*p)
{
    STREAMDB ts;
    BYTE mask[8]={0xFF},match[8];
    INT program_count=0;
    INT tunningIndex=0;
    INT searchByNIT=0;
    INT state=STATE_IDLE;
    DWORD fltPAT,fltSDT;
    struct timeval timeStart,timeEnd;//PSI/SI StartEnd time of TP search
    std::vector<DWORD>pmtfilters;
    SEARCHNOTIFY*notify=nullptr;
    match[0]=TBID_PAT;
    fltPAT=CreateFilter(PID_PAT,mask,match,1,&ts,false);
    match[0]=TBID_SDT;
    fltSDT=CreateFilter(PID_SDT,mask,match,1,&ts,false);
    SECTIONLIST nitsecs;//used for NIT searchtype;
    while(1){
        EPGMSG msg;
        memset(&msg,0,sizeof(msg));
        int rc=nglMsgQReceive(msgQ,&msg,sizeof(EPGMSG),1500);
        if(STATE_PSI_GETTING==state){
            gettimeofday(&timeEnd,NULL);//nglGetTime(&timeEnd);
            long tpms=(timeEnd.tv_sec-timeStart.tv_sec)*1000+(timeEnd.tv_usec-timeStart.tv_usec)/1000;
            if(tpms>3000){
                NGLOG_DEBUG("TPSearch used time %d",tpms); 
                SENDMSG(MSG_FINISHED_TP,program_count,0);
            }
        }
        if( ((state==STATE_IDLE) && (msg.msgid!=MSG_START_SEARCH)) )continue;
        switch(msg.msgid){
        case MSG_START_SEARCH: 
             NGLOG_VERBOSE("MSG_START_SEARCH");
             searchByNIT=(msg.param1==0);
             notify=(SEARCHNOTIFY*)msg.param2;
             tunningIndex=searchByNIT?-1:0;
             SENDMSG(MSG_TUNNING,0,0);
             state=STATE_STARTED;
             break;
        case MSG_TUNNING: //recv frequency request,the first tunning use param1=0,othercase param1!=0
             NGLOG_VERBOSE("MSG_TUNNING");
             state=STATE_TUNNING;
             tunningIndex=msg.param1;
             NGLOG_DEBUG("MSG_TUNNING freq[%d/%d]=%d",tunningIndex,schFrequencies.size(),schFrequencies[tunningIndex].u.c.frequency);
             if(schFrequencies[tunningIndex].delivery_type==DELIVERY_S||schFrequencies[tunningIndex].delivery_type==DELIVERY_S2)
                ConfigureTransponder(&schFrequencies[tunningIndex]); 
             if(NGL_OK!=nglTunerLock(0,&schFrequencies[tunningIndex])){
                 tunningIndex++;
                 SENDMSG(MSG_TUNNING,0,0);
             }
             break;
        case MSG_LOCK://lock status
             NGLOG_VERBOSE("MSG_LOCK");
             if(state!=STATE_IDLE){
                 if(msg.param1){//if frequency is locked
                     NGLOG_DEBUG("recv MSG_LOCK searchByNIT=%d",searchByNIT);
                     if(nitsecs.size()==0&&searchByNIT){
                         mask[0]=0xFF;match[0]=TBID_NIT;
                         CreateFilter(PID_NIT,mask,match,1,&nitsecs,true);
                     }else
                         SENDMSG(MSG_SEARCH_TP,0,0);
                 }else{//tuning failed;
                     SENDMSG(MSG_FINISHED_TP,0,tunningIndex);
                 }
             }
             break;
        case MSG_SEARCH_TP :
             NGLOG_VERBOSE("MSG_SEARCH_TP"); 
             ts.reset();
             ts.tune=schFrequencies[tunningIndex];
             nglStartSectionFilter(fltPAT);
             nglStartSectionFilter(fltSDT);
             program_count=0; 
             state=STATE_PSI_GETTING;
             if(notify&&notify->NEWTP)
                 notify->NEWTP(&schFrequencies[tunningIndex],0,0,notify->userdata);
             gettimeofday(&timeStart,NULL);//nglGetTime(&timeStart);
             break;
        case MSG_FINISHED_TP:
             NGLOG_VERBOSE("MSG_FINISHED_TP");
             std::sort(ts.pmt.begin(),ts.pmt.end(),[](PSITable&t1,PSITable&t2){return t1<t2;});//TableCompare);
             if(ts.sdt.size()){
                 SDT sdt(ts.sdt.back());
                 ts.netid=sdt.getNetId();
             }
             if(msg.param1)AddStreamDB(ts);
             NotifyServiceArrived(ts,notify);
             if(notify&&notify->FINISHTP)
                 notify->FINISHTP(&schFrequencies[tunningIndex],tunningIndex,schFrequencies.size(),notify->userdata);
             tunningIndex++;
             ts.reset();
             SENDMSG( ( (tunningIndex<schFrequencies.size() )?MSG_TUNNING:MSG_SEARCH_FINISHED ) ,tunningIndex,0);
             break;
        case MSG_SEARCH_FINISHED:
             NGLOG_DEBUG("SEARCH FINISHED!");
             tunningIndex=0;state=STATE_IDLE;
             state=STATE_IDLE;
             if(notify&&notify->FINISHTP)
                 notify->FINISHTP(nullptr,schFrequencies.size(),schFrequencies.size(),notify->userdata);
             memset(notify,0,sizeof(SEARCHNOTIFY));
             break;
        case MSG_PAT_RECEIVED:
             {  NGLOG_VERBOSE("MSG_PAT_RECEIVED"); 
                PAT pat(ts.pat.back(),false);
                MpegProgramMap maps[64];
                program_count=pat.getPrograms(maps,0);//dont care nit pid
                nglStopSectionFilter(msg.param2);
                for(int i=0;(program_count!=pmtfilters.size())&&(i<program_count);i++){
                    mask[0]=0xFF;  match[0]=0x02;mask[1]=mask[2]=0;
                    mask[3]=0xFF;  match[3]=maps[i].program_number>>8;
                    mask[4]=0xFF;  match[4]=maps[i].program_number&0xFF;
                    DWORD flt=CreateFilter(maps[i].pmt_pid,mask,match,5,&ts,true);
                    pmtfilters.push_back(flt);
                    //NGLOG_VERBOSE("\tPATService[%02d] id=%04x pmtpid=0x%04x flt=%x",i,maps[i].program_number,maps[i].pmt_pid,flt);
                }
             }
             break;
        case MSG_PMT_RECEIVED:
             {   
                NGLOG_VERBOSE("RECV MSG_PMT_RECEIVED pmt.size=%d/%d state=%d",ts.pmt.size(),program_count,state);
                nglFreeSectionFilter(msg.param2);
                if( ts.pmt.size()==program_count){
                    for(auto f:pmtfilters)nglFreeSectionFilter(f);
                    pmtfilters.clear();
                    if(ts.sdt.size()){
                         SDT sdt(ts.sdt.back());
                         if( ts.sdt.size()==sdt.lastSectionNo()+1){
                             state=STATE_PSI_GETTED;
                             SENDMSG(MSG_FINISHED_TP,program_count,0);//todo only call once per ts
                         }
                    }
                }
             }
             break;
        case MSG_SDT_RECEIVED:
             if(STATE_PSI_GETTING==state){
                int totalsvc=0;
                SDT sdt(ts.sdt.back());
                for(auto itr:ts.sdt){
                    SDT sdt(itr,false);
                    DVBService svs[32];
                    int sc=sdt.getServices(svs);
                    NGLOG_DEBUG("recv SDT sec[%d/%d]  services:%d pmt.size=%d",sdt.sectionNo(),ts.sdt.size(),sc,ts.pmt.size());
                    for(int i=0;i<sc;i++){
                        char sname[64];
                        svs[i].getServiceName(sname);
                        NGLOG_VERBOSE("\tSDTService id=0x%04x type=0x%02x camode=%d %s",svs[i].service_id,svs[i].serviceType,svs[i].freeCAMode,sname);
                    }totalsvc+=sc;
                }NGLOG_VERBOSE("sdt.size=%d sdt.svc.count=%d state=%d",ts.sdt.size(),totalsvc,state);
                if(ts.sdt.size()==sdt.lastSectionNo()+1)
                    nglStopSectionFilter(msg.param2);
                if(ts.pmt.size()==totalsvc&&(ts.sdt.size()==sdt.lastSectionNo()+1)){
                    program_count=totalsvc;
                    SENDMSG(MSG_FINISHED_TP,program_count,0);//todo only call once per ts
                    state=STATE_PSI_GETTED;
                }
             }break;
        case MSG_NIT_RECEIVED:
             {
                 NIT n1(nitsecs.back(),false);
                 STREAM streams[32];
                 NGLTunerParam tpinfo;
                 if(n1.lastSectionNo()+1<nitsecs.size())
                     break;
                 nglStopSectionFilter(msg.param2);
                 schFrequencies.clear();
                 for(auto itr:nitsecs){
                     NIT n2(itr);
                     int num=n2.getStreams(streams,false);
                     for(int i=0;i<num;i++){
                         if(streams[i].getDelivery(&tpinfo)){
                              NGLOG_DEBUG("[%d] freq=%d",schFrequencies.size(),tpinfo.u.c.frequency);
                              schFrequencies.push_back(tpinfo);
                         }  
                     }
                 }
                 tunningIndex=0;
                 SENDMSG(MSG_TUNNING,0,schFrequencies.size());
                 NGLOG_DEBUG("nit sections.size=%d frequency.size=%d",nitsecs.size(),schFrequencies.size());
             }break;
        default:break;
        }
    }
}

int DtvEpgInit(){
    BYTE mask;
    BYTE match;
    DWORD thid;
    if(0!=msgQ)
       return 0;
    nglDmxInit();
    nglTunerInit();
    nglAvInit();
    msgQ=nglMsgQCreate(10,sizeof(EPGMSG));

    mask=0xC0;match=0x4E;
    fltEIT=CreateFilter(PID_EIT,&mask,&match,1,(void*)fltEIT,true);

    mask=0xFE;  match=TBID_TDT;
    fltTDT=CreateFilter(PID_TDT,&mask,&match,1,(void*)fltTDT,true);

    mask=0xFF;  match=TBID_BAT;
    fltBAT=CreateFilter(PID_BAT,&mask,&match,1,NULL,true);
    nglCreateThread(&thid,0,4096,SearchProc,NULL);
    nglTunerRegisteCBK(0,TunningCBK,NULL);
}

void DtvSearch(const TRANSPONDER*tuningparams, int size, SEARCHNOTIFY*notify){
    EPGMSG msg;
    schFrequencies.clear();
    msg.param1=size;//0 for NIT search
    if(size==0)size++;
    schFrequencies.clear();
    for(int i=0;i<size;i++){
         schFrequencies.push_back(tuningparams[i]);
    }
    msg.msgid=MSG_START_SEARCH;
    msg.param2=(ULONG)notify;//
    nglMsgQSend(msgQ,&msg,sizeof(msg),1000);
}

static void NotifyServiceArrived(STREAMDB&ts,SEARCHNOTIFY*notify){
   NGLOG_VERBOSE("notify->SVC_CBK=%p",notify->SERVICE_CBK);
   if(notify&&notify->SERVICE_CBK)
       DtvEnumTSService(ts,notify->SERVICE_CBK,notify->userdata); 
}
