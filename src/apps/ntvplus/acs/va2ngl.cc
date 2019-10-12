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
#include <dvbepg.h>
NGL_MODULE(VA2NGL)

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
static SERVICELOCATOR lastplayed;
static ELEMENTSTREAM lastES[32];
static UINT num_elements=0;
static INT ACSHasStream(ELEMENTSTREAM*es,unsigned int numes,int pid){
    for(unsigned int i=0;i<numes;i++){
         if(es[i].pid==pid)return 1;
    }
    return 0;
}

static void CANOTIFY(UINT msg,const SERVICELOCATOR*svc,DWORD wp,ULONG lp,void*userdata){
    switch(msg){
    case MSG_SERVICE_CHANGE:
        {
            BYTE buffer[1024];
            int i,rc=DtvGetServicePmt(svc,buffer);
            PMT pmt(buffer,false);
            NGLOG_ERROR_IF(rc==0,"PMT not found");

            for(i=0;i<num_elements;i++){
               VA_CTRL_RemoveStream(lastES[i].pid);
               NGLOG_DEBUG("SERVICE_CHANGE::VA_CTRL_RemoveStream %d",lastES[i].pid);
            }
            VA_CTRL_SwitchOffProgram(0);
            if(svc->netid!=lastplayed.netid||svc->tsid!=lastplayed.tsid||(svc->tpid!=lastplayed.tpid)){
                VA_CTRL_TsChanged(0);
                NGLOG_DEBUG("VA_CTRL_TsChanged");
            }
            lastplayed=*svc;
            NGLOG_DUMP("VA_CTRL_SwitchOnProgram::PMT",buffer,8);
            VA_CTRL_SwitchOnProgram(0,pmt.sectionLength()+3,buffer);
            num_elements=pmt.getElements(lastES,false);
            NGLOG_DEBUG("SERVICE_CHANGE::PLAY %d.%d.%d pmtlen=%d  %d elements",svc->netid,svc->tsid,svc->sid,pmt.sectionLength(),num_elements);
            for(i=0;i<num_elements;i++){
                VA_CTRL_AddStream(0,lastES[i].pid,lastES[i].pid,ACSStreamNOTIFY);
                NGLOG_DEBUG("SERVICE_CHANGE::VA_CTRL_AddStream [%d] type=%d pid=%d",i,lastES[i].stream_type,lastES[i].pid);
            }
        }break;
    case MSG_PMT_CHANGE:
        {
            int nes;
            ELEMENTSTREAM newes[32];
            PMT pmt((BYTE*)lp,false);
            VA_CTRL_PmtUpdated(0,wp,(BYTE*)lp);
            nes=pmt.getElements(newes,false);
            for(int i=0;i<num_elements;i++){
                if(0==ACSHasStream(newes,nes,lastES[i].pid)){
                    VA_CTRL_RemoveStream(lastES[i].pid);
                    NGLOG_DEBUG("VA_CTRL_RemoveStream %d",lastES[i].pid);
                }
            }
            for(int i=0;i<nes;i++){
                int has=ACSHasStream(lastES,num_elements,newes[i].pid);
                if(0==has)
                    VA_CTRL_AddStream(0,newes[i].pid,newes[i].pid,ACSStreamNOTIFY);
                else
                    VA_CTRL_UpdateStream(newes[i].pid,newes[i].pid);
                NGLOG_DEBUG("VA_CTRL_%sStream[%d] %d",(has?"Update":"Add"),i,newes[i].pid);
            }
            memcpy(lastES,newes,sizeof(lastES));
            NGLOG_DEBUG("MSG_PMT_CHANGED %d.%d.%d ver:%d elements %d->%d",svc->netid,svc->tsid,svc->sid,pmt.version(),num_elements,nes);
            num_elements=nes;
        }break;
    case MSG_CAT_CHANGE:
        {
            CAT cat((BYTE*)lp,false);
            NGLOG_DEBUG("MSG_CAT_CHANGE len:%d ver:%d",wp,cat.version());
            VA_CTRL_CatUpdated(0,wp,(BYTE*)lp);
        }break;
    default:break;
    }
}

void  StartACS(){
    int i;
    pthread_t tid;
    VA_DSCR_Init();
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
