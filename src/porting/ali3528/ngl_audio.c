#include <aui_dmx.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_snd.h>
#include <ngl_types.h>
#include <ngl_log.h>

NGL_MODULE(AUDIO)

static aui_hdl snd_hdl;
INT nglSndInit(){
    aui_attr_snd attr_snd;
    MEMSET(&attr_snd,0,sizeof(aui_attr_snd));
    aui_snd_init(NULL,NULL);
    if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl)) {
        if (aui_snd_open(&attr_snd,&snd_hdl)) {
            NGLOG_DEBUG("\n aui_snd_open fail\n");
            return -1;
        }
    }
    return NGL_OK;
}

INT nglSndSetVolume(int idx,int vol){
    aui_snd_vol_set(snd_hdl,vol);
    return NGL_OK;
}
 
INT nglSndSetMute(int idx,BOOL mute){
    return aui_snd_mute_set(snd_hdl,mute);
}

INT nglSndSetSPDIF(int idx){
}

