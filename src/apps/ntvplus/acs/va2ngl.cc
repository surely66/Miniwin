#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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

static BYTE CATSection[1024];
static INT SectionCBK(DWORD dwVaFilterHandle,UINT32 uiBufferLength,BYTE *pBuffer, void *pUserData)
{
    if(1==pBuffer[0]){
        if(memcmp(pBuffer,CATSection,8)){
            NGLOG_DEBUG("CAT Updated SectionCBK filter=0x%x tbid=%x",dwVaFilterHandle,pBuffer[0] );
            VA_CTRL_CatUpdated(0,uiBufferLength,pBuffer);     
            memcpy(CATSection,pBuffer,uiBufferLength);
        }
    }
}
static DWORD CreateFilter(int pid,int tbid){
    DWORD  hFilter;
    BYTE mask[8],value[8];
    mask[0]=0xFF;value[0]=tbid;
    hFilter=VA_DMX_AllocateSectionFilter(0,0,pid,SectionCBK);
    VA_DMX_SetSectionFilterParameters(hFilter,1,mask,value,eVA_DMX_Continuous);
    VA_DMX_StartSectionFilter(hFilter);
    return hFilter;
}

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
    CreateFilter(1,1);
    VA_CTRL_Start();
    NGLOG_DEBUG("VA_CTRL_Start...end");
}

static SERVICELOCATOR lastplayed;
static WORD lastpids[32];
static void CANOTIFY(UINT msg,const SERVICELOCATOR*svc,void*userdata){
    BYTE buffer[1024];
    int i,ne,rc;
    ELEMENTSTREAM es[32];
    if(MSG_SERVICE_CHANGE!=msg)
        return;
    rc=DtvGetServicePmt(svc,buffer);
    NGLOG_ERROR_IF(rc==0,"PMT not found");
    PMT pmt(buffer,false);
    if(svc->netid!=lastplayed.netid||svc->tsid!=lastplayed.tsid){
        VA_CTRL_TsChanged(0);VA_CTRL_TsChanged(1);
    }
    lastplayed=*svc;

    VA_CTRL_SwitchOffProgram(0);
    NGLOG_DUMP("PMT",buffer,8);
    VA_CTRL_SwitchOnProgram(0,pmt.sectionLength()+3,buffer);
    VA_CTRL_SwitchOnProgram(1,pmt.sectionLength()+3,buffer);
    for(i=0;i<32&&lastpids[i]!=0;i++)
        VA_CTRL_RemoveStream(lastpids[i]);
    ne=pmt.getElements(es,false);
    memset(lastpids,0,sizeof(lastpids));
    NGLOG_DEBUG("DtvGetServicePmt=%d PLAY %d.%d.%d pmtlen=%d  %d elements",rc,svc->netid,svc->tsid,svc->sid,pmt.sectionLength(),ne);
    for(i=0;i<ne;i++){
        NGLOG_VERBOSE("[%d] type=%d pid=%d",i,es[i].stream_type,es[i].pid);
        VA_CTRL_AddStream(0,es[i].pid,es[i].pid,NULL);
        VA_CTRL_AddStream(1,es[i].pid,es[i].pid,NULL);
        lastpids[i]=es[i].pid;
    }
}

void  StartACS(){
    int i;
    pthread_t tid;
    VA_DSCR_Init();
    pthread_create(&tid,NULL,ACSProc,NULL);
    DtvRegisterNotify(CANOTIFY,NULL);
}  
