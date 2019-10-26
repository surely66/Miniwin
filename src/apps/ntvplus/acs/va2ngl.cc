#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include<stdarg.h>
extern "C"{
#include <va_types.h>
#include <va_ctrl.h>
#include <va_os.h>
#include <va_sc.h>
#include <va_dmx.h>
#include <va_xnvm.h>
DWORD VA_DSCR_Init();
}
#include <ngl_video.h>
#include <ngl_log.h>
#include <ngl_tuner.h>
#include <ngl_pvr.h>
#include <ngl_dmx.h>
#include <dvbepg.h>
NGL_MODULE(VA2NGL)

static SERVICELOCATOR lastplayed;
static DWORD flt_pmt=0;
static DWORD flt_cat=0;
static BYTE CATSection[1024];
static BYTE PMTSection[1024];
static UINT num_elements=0;
static ELEMENTSTREAM elements[16];

static void*ACSProc(void*p){
    int i,ret;
    tVA_CTRL_ConfigurationParameters param;
    memset(&param,0,sizeof(tVA_CTRL_ConfigurationParameters));
    param.uiNbAcs = kVA_SETUP_NBMAX_ACS ;//1
    param.uiNbSmartCard = kVA_SETUP_NBMAX_SC ;//1
    param.aAcs->stDemux.uiMaxNumberOfFilters = 16;
    param.aAcs->stDemux.uiMaxSizeOfFilter = 4096;
    param.aAcs->stDescrambler.uiMaxNumberOfChannels =4;
    //param.stStbInformation.
    NGLOG_DEBUG("VA_CTRL_Init calling...");
    VA_DSCR_Init();
    ret = VA_CTRL_Init(&param);
    NGLOG_DEBUG("VA_CTRL_Init=%d",ret);
    NGLOG_DEBUG("VA_CTRL_Start...");
    for(i=0;i<3;i++)
        VA_CTRL_OpenAcsInstance(i,(tVA_CTRL_AcsMode)i);

    VA_CTRL_Start();
    NGLOG_DEBUG("VA_CTRL_Start...end");
}

static INT ACSStreamNOTIFY(DWORD dwStbStreamHandle, tVA_CTRL_StreamNotificationType eStreamNotificationType, tVA_CTRL_StreamNotificationInfo uStreamNotificationInfo){
    return kVA_OK;
}
static INT ACSHasStream(ELEMENTSTREAM*es,unsigned int numes,int pid){
    for(unsigned int i=0;i<numes;i++){
         if(es[i].pid==pid)return 1;
    }
    return 0;
}

static void SwitchProgram(BYTE*pmtbuf,UINT pmtlen){
    unsigned int i,rc;
    unsigned short num_new_elements;
    ELEMENTSTREAM new_elements[16];
    PMT pmt(pmtbuf,FALSE);
    PMT pmt0(PMTSection,FALSE);
    num_new_elements=pmt.getElements(new_elements,false);

    if(pmt0.getProgramNumber()==pmt.getProgramNumber()){//update program
       rc=VA_CTRL_PmtUpdated(0,pmtlen,pmtbuf);
       NGLOG_DEBUG("VA_CTRL_PmtUpdated=%d serviceid=%d pidcount=%d/%d",rc,pmt.getProgramNumber(),num_elements,num_new_elements);
       for(i=0;(i<num_elements);i++){//remove elements which not contained in new pmt
          if(ACSHasStream(new_elements,num_new_elements,elements[i].pid)==0){
             VA_CTRL_RemoveStream(elements[i].pid);
             NGLOG_DEBUG("VA_CTRL_RemoveStream[%d] %d",i,elements[i].pid);
          }else{
             VA_CTRL_UpdateStream(elements[i].pid,elements[i].pid);
             NGLOG_DEBUG("VA_CTRL_UpdateStream[%d] %d",i,elements[i].pid);
          }
       }
       for(i=0;i<num_new_elements;i++){
          if(ACSHasStream(elements,num_elements,new_elements[i].pid)==0){
             VA_CTRL_AddStream(0,new_elements[i].pid,new_elements[i].pid,ACSStreamNOTIFY);
             NGLOG_DEBUG("VA_CTRL_AddStream[%d] %d",i,new_elements[i].pid);
          }
       }
    }else{//switch program
       rc=VA_CTRL_SwitchOnProgram(0,pmtlen,pmtbuf);
       NGLOG_DEBUG("VA_CTRL_SwitchOnProgram=%d elements=%d/%d pmt=%p pmtlen=%d service=%d/%d",rc,num_elements,num_new_elements,
              pmt,pmtlen, pmt0.getProgramNumber(),pmt.getProgramNumber());
       for(i=0;i<num_new_elements;i++){//add new elements(of new pmt) which not contain in old pmt
          VA_CTRL_AddStream(0,new_elements[i].pid,new_elements[i].pid,ACSStreamNOTIFY);
          NGLOG_DEBUG("VA_CTRL_AddStream %d",new_elements[i].pid);
       }
    }
    memcpy(elements,new_elements,sizeof(new_elements));
    num_elements=num_new_elements;
}

static void SectionCBK(DWORD dwVaFilterHandle,const BYTE *Buffer, unsigned int uiBufferLength,void *pUserData){
    PSITable si(Buffer,0);
    PSITable cat(CATSection,0);
    PSITable pmt(PMTSection,0);
    switch(Buffer[0]){
    case 1:/*CAT*/
         if(cat.version()!=si.version()){
            NGLOG_DUMP("CAT",Buffer,32);
            VA_CTRL_CatUpdated(0,uiBufferLength,(BYTE*)Buffer);
            memcpy(CATSection,Buffer,uiBufferLength);
         }
         break;
    case 2:/*PMT*/
         if(pmt.version()!=si.version()){
             NGLOG_DUMP("PMT",Buffer,32);
             SwitchProgram((BYTE*)Buffer,uiBufferLength);
             memcpy(PMTSection,Buffer,uiBufferLength);
         }
         break;
    default:break;
    }
}

static DWORD CreateFilter(int pid,int num,...){
    va_list ap;
    DWORD  hFilter;
    BYTE mask[16],value[16];
    int i,idx;
    char buffer[64];
    bzero(mask,sizeof(mask));
    bzero(value,sizeof(mask));
    bzero(buffer,sizeof(buffer));
    va_start(ap,num);
    for(i=0,idx=0;i<num;i++){
        mask[idx]=0xFF;
        value[idx]=(BYTE)va_arg(ap,int);
        idx+=((i==0)*2+1);
    }
    va_end(ap);
    for(i=0;i<idx;i++)sprintf(buffer+i*3,"%02x,",value[i]);
    if(num>1)num+=2;
    hFilter=nglAllocateSectionFilter(0,pid,SectionCBK,NULL,NGL_DMX_SECTION);
    nglSetSectionFilterParameters(hFilter,mask,value,num);//0-onshort ,1-eVA_DMX_Continuous);
    nglStartSectionFilter(hFilter);
    return hFilter;
}

static void CANOTIFY(UINT msg,const SERVICELOCATOR*svc,DWORD wp,ULONG lp,void*userdata){
    switch(msg){
    case MSG_SERVICE_CHANGE:
        {
            int pmtpid=DtvGetServerPmtPid(svc);
            nglStopSectionFilter(flt_pmt);
            nglFreeSectionFilter(flt_pmt);
            for(int i=0;i<num_elements;i++){
               VA_CTRL_RemoveStream(elements[i].pid);
               NGLOG_DEBUG("VA_CTRL_RemoveStream %d",elements[i].pid);
            }
            VA_CTRL_SwitchOffProgram(0);
            if(svc->netid!=lastplayed.netid||svc->tsid!=lastplayed.tsid||(svc->tpid!=lastplayed.tpid)){
                VA_CTRL_TsChanged(0);
                NGLOG_DEBUG("VA_CTRL_TsChanged");
                nglStopSectionFilter(flt_cat);
                nglFreeSectionFilter(flt_cat);
                flt_cat=CreateFilter(1,1,1);
            }
            flt_pmt=CreateFilter(pmtpid,3,0x02,(svc->sid>>8),(svc->sid&0xFF));
        }break;
    case MSG_PMT_CHANGE:
    case MSG_CAT_CHANGE:
    default:break;
    }
}

void  StartACS(){
    pthread_t tid;
    pthread_create(&tid,NULL,ACSProc,NULL);
    DtvRegisterNotify(CANOTIFY,NULL);
}

extern "C" INT GetCurrentServiceEcmPids(USHORT*espids){
    INT i,j,cnt,nes;
    USHORT*ppid=espids;
    USHORT tsecmpid=0x1FFF,tscaid=0xFFFF;
    BYTE buffer[1024];
    SERVICELOCATOR svc;
    ELEMENTSTREAM es[32];
    USHORT caids[16],ecmpids[16];
    PMT pmt(buffer,false);
    DtvGetCurrentService(&svc);
    DtvGetServicePmt(&svc,buffer);
    cnt=pmt.getCAECM(caids,ecmpids);
    for(i=0;i<cnt;i++){
         if(caids[i]==0x500){
             tscaid=caids[i];
             tsecmpid=ecmpids[i];
             break;
         }
    }
    nes=pmt.getElements(es,false);
    for(i=0;i<nes;i++){
        USHORT escaid=0xFFFF,esecmpid=0x1FFF;
        cnt=es[i].getCAECM(caids,ecmpids);
        if(tsecmpid!=0x1FFF||tscaid!=0xFFFF){//ts scrambled
            escaid=tscaid;esecmpid=tscaid;
        }
        if(cnt){
            escaid=caids[0];esecmpid=ecmpids[0];
        }
        if(escaid==tscaid||esecmpid==tscaid)
            *ppid++=es[i].pid; 
    }
    return ppid-espids;
}  
