#include <ngl_types.h>
#include <ngl_pvr.h>
#include <aui_pvr.h>
#include <aui_ca_pvr.h>
#include <aui_dsc.h>
#include <ngl_log.h>
#include <ngl_timer.h>


NGL_MODULE(PVR)

static unsigned char*pvr_buffer=NULL;
typedef struct{
   aui_hdl hdl;
   char *path;
}NGLPVR;

DWORD nglPvrInit(){
    unsigned int pvr_buffer_len = 20*1024*1024;
    if(NULL!=pvr_buffer){//already inited
       return 0;
    }
    pvr_buffer = (unsigned char *)malloc(pvr_buffer_len);
    if(pvr_buffer == NULL){
 	NGLOG_ERROR("pvr_buffer is %p",pvr_buffer);
	return NGL_ERROR;
    }
    
    aui_pvr_init_param param;
    memset(&param,0,sizeof(param));
    param.max_rec_number = 2;
    param.max_play_number =1 ;
    param.ac3_decode_support = 1;
    param.continuous_tms_en = 0;
    param.debug_level   = AUI_PVR_DEBUG_ALL;
    STRCPY(param.dvr_path_prefix,"NGLDVR/");
    STRCPY(param.info_file_name,"info.dvr");	
    STRCPY(param.info_file_name_new,"info3.dvr");
    STRCPY(param.ts_file_format,"dvr");	
    STRCPY(param.ts_file_format_new, "ts");	
    STRCPY(param.ps_file_format,"mpg");	
    STRCPY(param.test_file1,"test_write1.dvr");
    STRCPY(param.test_file2,"test_write2.dvr");
    STRCPY(param.storeinfo_file_name,"storeinfo.dvr");
    param.record_min_len = 15;		// in seconds, recomment to 15s, the record will be deleted if shorter that this limit
    param.tms_time_max_len = 7200;	// in seconds, recomment to 2h(7200);
    param.tms_file_min_size = 2;	// in MBytes,  recomment to 10M
    param.prj_mode  = AUI_PVR_DVBS2; 
    param.cache_addr = (unsigned int)pvr_buffer;
    param.cache_size = pvr_buffer_len;

    int rc=aui_pvr_init(&param);
    NGLOG_DEBUG("aui_pvr_init=%d",rc);
    aui_log_priority_set(AUI_MODULE_PVR,AUI_LOG_PRIO_DEBUG);

    aui_pvr_disk_attach_info p_apart_param;
    MEMSET(&p_apart_param, 0, sizeof(aui_pvr_disk_attach_info));
    STRCPY(p_apart_param.mount_name,"/mnt/nfs");
    p_apart_param.disk_usage = AUI_PVR_REC_AND_TMS_DISK;
    p_apart_param.sync = 1;
    p_apart_param.init_list = 1;
    p_apart_param.check_speed = 1;
    rc=aui_pvr_disk_attach(&p_apart_param); 
    NGLOG_DEBUG("aui_pvr_disk_attach=%d",rc);
}

static void ali_aui_pvr_callback(aui_hdl handle, unsigned int msg_type, unsigned int msg_code, void* user_data){
    unsigned int index;
    aui_pvr_pid_info aui_pid_info;
    aui_pvr_rec_item_info rec_info;
    NGLOG_VERBOSE("handle=%p msg_type=%d msg_code=%x",handle,msg_type,msg_code);
    switch(msg_type){
    case AUI_EVNT_PVR_END_DATAEND:
    case AUI_EVNT_PVR_END_DISKFULL:
    case AUI_EVNT_PVR_END_TMS:
    case AUI_EVNT_PVR_END_REVS:
    case AUI_EVNT_PVR_END_WRITEFAIL:
    case AUI_EVNT_PVR_END_READFAIL:
    case AUI_EVNT_PVR_TMS_OVERLAP:
    case AUI_EVNT_PVR_STATUS_UPDATE:/*13*/ break;
    case AUI_EVNT_PVR_STATUS_FILE_CHG:
    case AUI_EVNT_PVR_STATUS_PID_CHG:
    case AUI_EVNT_PVR_SPEED_LOW:
    case AUI_EVNT_PVR_STATUS_CHN_CHG:
    case AUI_EVNT_PVR_MSG_REC_START:/*18*/
    case AUI_EVNT_PVR_MSG_REC_STOP:

    case AUI_EVNT_PVR_MSG_PLAY_START:
    case AUI_EVNT_PVR_MSG_PLAY_STOP:
    case AUI_EVNT_PVR_MSG_UPDATE_KEY:
    case AUI_EVNT_PVR_MSG_UPDATE_CW:
    case AUI_EVNT_PVR_MSG_TMS_CAP_UPDATE:
    
    break;		
    }
}

static char lastPVR[512];

DWORD nglPvrRecordOpen(const char*record_path,const NGLPVR_RECORD_PARAM*param){
    unsigned int rc, i = 0;
    aui_hdl aui_pvr_handler=NULL;
    AUI_RTN_CODE ret= AUI_RTN_SUCCESS;
    aui_record_prog_param st_arp;
    aui_ca_pvr_callback ca_pvr_callback;

    nglPvrInit();
    #ifdef AUI_LINUX
    aui_ca_pvr_config config;
    memset(&config,0,sizeof(aui_ca_pvr_config));
    #endif
    memset(&st_arp,0,sizeof(st_arp));
    
    st_arp.dmx_id=0;
    st_arp.is_tms_record=param->recordMode;
    st_arp.av_flag=param->video_pid!=NGL_INVALID_PID;
    
    st_arp.h264_flag = param->video_type;
    st_arp.fn_callback = ali_aui_pvr_callback;
    st_arp.pid_info.video_pid =param->video_pid;
    st_arp.pid_info.pcr_pid =param->pcr_pid;

    NGLOG_DEBUG("videopid=%d pcr_pid=%d  recodemode=%d",param->video_pid,param->pcr_pid,param->recordMode);
    for(i = 0; i < PVR_MAX_AUDIO; i++) {
        int idx=st_arp.pid_info.audio_count;
        st_arp.pid_info.audio_pid[idx] = param->audio_pids[i];
	st_arp.pid_info.audio_type[idx] =param->audio_types[i];
        if((param->audio_pids[idx]!=NGL_INVALID_PID)&&(param->audio_pids[idx]!=0)){
           NGLOG_DEBUG("audiopid[%d]=%d",st_arp.pid_info.audio_count,param->audio_pids[i]);
           st_arp.pid_info.audio_count++;
        }
    }

    st_arp.ca_mode=0;//0--free 1 ca scrambled
    st_arp.rec_type = AUI_PVR_REC_TYPE_TS;    
    st_arp.is_reencrypt = 0;
    st_arp.rec_special_mode = AUI_PVR_NONE;
    st_arp.is_scrambled = 0;

    if(NULL==record_path){
        char fname[64];
        NGL_TIME tnow;
        NGL_TM tmn;
        nglGetTime(&tnow);
        nglTimeToTm(&tnow,&tmn);
        sprintf(fname,"PVR_%d-%d-%d_%02d%02d%02d",1900+tmn.uiYear,tmn.uiMonth,tmn.uiMonthDay,tmn.uiHour,tmn.uiMin,tmn.uiSec);
        strcpy(st_arp.folder_name,fname);      
    }else
        strcpy(st_arp.folder_name,record_path);
    strcpy(lastPVR,st_arp.folder_name);
    rc=aui_pvr_rec_open(&st_arp,&aui_pvr_handler);
    NGLOG_INFO("aui_pvr_rec_open=%d path=%s  aui_pvr_handler=%p",rc,st_arp.folder_name,aui_pvr_handler);
    if(0==rc){ 
        NGLPVR*pvr=(NGLPVR*)nglMalloc(sizeof(NGLPVR));
        pvr->hdl=aui_pvr_handler;
        pvr->path=strdup(st_arp.folder_name);
        return pvr;
    }
    return 0;
}

DWORD nglPvrRecordPause(DWORD handler){
    UINT duration;
    NGLPVR*pvr=(NGLPVR*)handler;
    INT ret = aui_pvr_rec_state_change(pvr->hdl,AUI_PVR_REC_STATE_PAUSE);
    aui_pvr_get(handler,AUI_PVR_REC_TIME_S,&duration,0,0);
    NGLOG_DEBUG("******pvr reocrd pause at [%d]",duration);
    return NGL_OK;
}

DWORD nglPvrRecordResume(DWORD handler){
    UINT duration;
    NGLPVR*pvr=(NGLPVR*)handler;
    INT ret = aui_pvr_rec_state_change(pvr->hdl,AUI_PVR_REC_STATE_RECORDING);
    aui_pvr_get(pvr->hdl,AUI_PVR_REC_TIME_S,&duration,0,0);
    NGLOG_DEBUG("************pvr reocrd resume at [%d]",duration);
    return NGL_OK;
}

DWORD nglPvrRecordClose(DWORD handler){
    int rc;
    aui_pvr_stop_ply_param st_apsp;
    st_apsp.stop_mode = AUI_PVR_STOPPED_ONLY;
    st_apsp.sync =TRUE;
    st_apsp.vpo_mode=0;
    NGLPVR*pvr=(NGLPVR*)handler;
    rc= aui_pvr_ply_close(pvr->hdl,&st_apsp);
    pvr->hdl=NULL;
    nglFree(pvr->path);
    nglFree(pvr);
    NGLOG_DEBUG("aui_pvr_ply_close %p=%d",handler,rc);
    return (AUI_RTN_SUCCESS==rc)?NGL_OK:NGL_ERROR;
}

void nglGetPvrPath(DWORD handler,char*path){
    NGLPVR*pvr=(NGLPVR*)handler;
    NGLOG_DEBUG("handle=%p",handler);
    strcpy(path,"/mnt/nfs/NGLDVR/");
    strcat(path,pvr->path);
}

///////////////////////////////PVR PLAYER////////////////////////////

DWORD nglPvrPlayerOpen(const char*pvrpath){
    int ret;
    aui_hdl hdl=NULL;
    aui_ply_param st_app;
    MEMSET(&st_app,0,sizeof(st_app));
    st_app.dmx_id =2;
    st_app.index =0;
    st_app.live_dmx_id =0;
    if(NULL==pvrpath){
        strcpy(st_app.path,"/mnt/nfs/NGLDVR/");
        strcat(st_app.path,lastPVR);
    }else
        strcpy(st_app.path,pvrpath);//"/mnt/uda1/ALIDVRS2/[TS]2013-11-01-13-32-30");
    st_app.preview_mode =0;
    st_app.speed  = AUI_PVR_PLAY_SPEED_1X;
    st_app.start_mode = 4;
    st_app.start_pos =0 ;
    st_app.start_time =0;
    st_app.state = AUI_PVR_PLAY_STATE_STEP;
    st_app.fn_callback=ali_aui_pvr_callback;
    ret = aui_pvr_ply_open(&st_app,&hdl);
    NGLOG_DEBUG("aui_pvr_ply_open=%d handle=%p",ret,hdl);
    return hdl;
}

DWORD nglPvrPlayerPlay(DWORD handle){
    int ret=aui_pvr_ply_state_change((aui_hdl)handle,AUI_PVR_PLAY_STATE_PLAY,0); 
    NGLOG_DEBUG("aui_pvr_ply_state_change=%d handle=%p",ret,handle);
    return ret==AUI_RTN_SUCCESS?NGL_OK:NGL_ERROR;
}

DWORD nglPvrPlayerStop(DWORD handle){
    int ret=aui_pvr_ply_state_change((aui_hdl)handle,AUI_PVR_PLAY_STATE_STOP,0);
    NGLOG_DEBUG("aui_pvr_ply_state_change=%d handle=%p",ret,handle);
    return ret==AUI_RTN_SUCCESS?NGL_OK:NGL_ERROR;
}

DWORD nglPvrPlayerPause(DWORD handle){
    int ret=aui_pvr_ply_state_change((aui_hdl)handle,AUI_PVR_PLAY_STATE_PAUSE,0);
    NGLOG_DEBUG("aui_pvr_play_state_chage=%d handle=%p",ret,handle);
    return ret==AUI_RTN_SUCCESS?NGL_OK:NGL_ERROR;
}

DWORD nglPvrPlayerClose(DWORD handle){
    int ret;
    aui_pvr_stop_ply_param st_apsp;
    st_apsp.stop_mode = AUI_PVR_STOPPED_ONLY;
    st_apsp.sync =TRUE;
    st_apsp.vpo_mode=0;
    ret = aui_pvr_ply_close((aui_hdl)handle,&st_apsp);
    NGLOG_DEBUG("aui_pvr_ply_close=%d handle=%p",ret,handle);
    return ret==AUI_RTN_SUCCESS?NGL_OK:NGL_ERROR;
}

