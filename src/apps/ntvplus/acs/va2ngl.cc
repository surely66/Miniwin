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

static SERVICELOCATOR lastplayed;
static WORD lastpids[32];
static INT ACSStreamNOTIFY(DWORD dwStbStreamHandle, tVA_CTRL_StreamNotificationType eStreamNotificationType, tVA_CTRL_StreamNotificationInfo uStreamNotificationInfo){
    return kVA_OK;
}
static void CANOTIFY(UINT msg,const SERVICELOCATOR*svc,DWORD wp,ULONG lp,void*userdata){
    BYTE buffer[1024];
    int i,ne,rc;
    ELEMENTSTREAM es[32];
    PMT pmt(buffer,false);
    switch(msg){
    case MSG_SERVICE_CHANGE:
        rc=DtvGetServicePmt(svc,buffer);
        NGLOG_ERROR_IF(rc==0,"PMT not found");

        for(i=0;i<32&&lastpids[i]!=0;i++){
           VA_CTRL_RemoveStream(lastpids[i]);
        }
        VA_CTRL_SwitchOffProgram(0);
        if(svc->netid!=lastplayed.netid||svc->tsid!=lastplayed.tsid||(svc->tpid!=lastplayed.tpid)){
           VA_CTRL_TsChanged(0);//VA_CTRL_TsChanged(1);
        }
        lastplayed=*svc;
        //VA_CTRL_SwitchOffProgram(1);
        NGLOG_DUMP("PMT",buffer,8);
        VA_CTRL_SwitchOnProgram(0,pmt.sectionLength()+3,buffer);
        //VA_CTRL_SwitchOnProgram(1,pmt.sectionLength()+3,buffer);
        ne=pmt.getElements(es,false);
        memset(lastpids,0,sizeof(lastpids));
        NGLOG_DEBUG("DtvGetServicePmt=%d PLAY %d.%d.%d pmtlen=%d  %d elements",rc,svc->netid,svc->tsid,svc->sid,pmt.sectionLength(),ne);
        for(i=0;i<ne;i++){
            NGLOG_DEBUG("[%d] type=%d pid=%d",i,es[i].stream_type,es[i].pid);
            VA_CTRL_AddStream(0,es[i].pid,es[i].pid,ACSStreamNOTIFY);
            //VA_CTRL_AddStream(1,es[i].pid,es[i].pid,NULL);
            lastpids[i]=es[i].pid;
        }
        break;
    case MSG_PMT_CHANGE:
        {
            PSITable table((BYTE*)lp,false);
            NGLOG_DEBUG("MSG_PMT_CHANGED %d.%d.%d sid:%x ver:%d",svc->netid,svc->tsid,svc->sid,table.extTableId(),table.version());
        }break;
    case MSG_CAT_CHANGE:
        {
            PSITable table((BYTE*)lp,false);
            NGLOG_DEBUG("MSG_CAT_CHANGE ver:%d len:%d",table.version(),wp);
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
