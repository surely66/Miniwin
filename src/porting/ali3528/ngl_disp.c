#include<aui_dis.h>
#include<ngl_types.h>
#include<ngl_log.h>
#include<ngl_disp.h>

NGL_MODULE(DISP)

static aui_hdl dis_hd;
static aui_hdl dis_sd;

DWORD nglDispInit(){
    int rc;
    aui_attr_dis attr_dis;
    MEMSET(&attr_dis, 0 ,sizeof(aui_attr_dis));

    NGLOG_DEBUG("\n init_dis_handle(),111\n");	
	
    if (aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_HD, &dis_hd)){
        attr_dis.uc_dev_idx = AUI_DIS_HD;
	rc=aui_dis_open(&attr_dis, &dis_hd);
	NGLOG_DEBUG("aui_dis_open HD =%d",rc);
    }

    NGLOG_DEBUG("\n init_dis_handle(),222\n");	
	
    if (aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_SD, &dis_sd)){
	attr_dis.uc_dev_idx = AUI_DIS_SD;
	rc=aui_dis_open(&attr_dis, &dis_sd);
   	NGLOG_DEBUG(" aui_dis_open SD =%d",rc);
    }
}

DWORD nglDispSetAspectRatio(int r){
    int ratio;
    switch(ratio){
    case DISP_APR_AUTO:
         ratio=AUI_DIS_AP_AUTO;break;
    case DISP_APR_4_3:
         ratio=AUI_DIS_AP_4_3;break;
    case DISP_APR_16_9:
         ratio=AUI_DIS_AP_16_9;break;
    default:return NGL_ERROR;
    }    
    return aui_dis_aspect_ratio_set(dis_hd,ratio);
}

DWORD nglDispGetAspectRatio(int*ratio){
    struct aui_dis_info dis_info;
    memset(&dis_info,0,sizeof(aui_dis_info));
    aui_dis_get((void*)dis_hd,AUI_DIS_GET_INFO,(void*)&dis_info);
    switch(dis_info.dis_aspect_ratio){
    case AUI_DIS_AP_AUTO: *ratio=DISP_APR_AUTO; break;
    case AUI_DIS_AP_4_3:  *ratio=DISP_APR_4_3;  break;
    case AUI_DIS_AP_16_9: *ratio= DISP_APR_16_9;break;
    default:return NGL_ERROR;
    }
    return NGL_OK;
}

DWORD nglDispSetMatchMode(int md){
    aui_dis_match_mode mm;
    switch(md){
    case DISP_MM_PANSCAN : mm=AUI_DIS_MM_PANSCAN;break;
    case DISP_MM_LETTERBOX:mm=AUI_DIS_MM_LETTERBOX;break;
    case DISP_MM_PILLBOX : mm=AUI_DIS_MM_PILLBOX;break;
    case DISP_MM_NORMAL_SCALE:mm=AUI_DIS_MM_NORMAL_SCALE;break;
    case DISP_COMBINED_SCALE:mm=AUI_DIS_MM_COMBINED_SCALE;break;
    return NGL_ERROR;
    }
    return aui_dis_match_mode_set(dis_hd,mm);
}

DWORD nglDispSetTvSystem(){
    
}
