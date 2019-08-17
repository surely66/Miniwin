#include<ngl_types.h>
#include<ngl_log.h>
#include<ngl_disp.h>

NGL_MODULE(DISP)

DWORD nglDispInit(){
    NGLOG_DEBUG("\n init_dis_handle(),111\n");	
}

DWORD nglDispSetAspectRatio(int r){
    int ratio;
    switch(ratio){
    case DISP_APR_AUTO:
    case DISP_APR_4_3:
    case DISP_APR_16_9:
    default:return NGL_ERROR;
    }    
    return NGL_OK;
}

DWORD nglDispGetAspectRatio(int*ratio){
    return NGL_OK;
}

DWORD nglDispSetMatchMode(int md){
    return NGL_ERROR;
}

DWORD nglDispSetTvSystem(){
    
}
