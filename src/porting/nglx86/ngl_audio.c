#include <ngl_types.h>
#include <ngl_log.h>

NGL_MODULE(AUDIO)

INT nglSndInit(){
    return NGL_OK;
}

INT nglSndSetVolume(int idx,int vol){
    return NGL_OK;
}
 
INT nglSndSetMute(int idx,BOOL mute){
    return NGL_OK;//aui_snd_mute_set(snd_hdl,mute);
}

INT nglSndSetOutput(int dev,int type){
    //aui_snd_out_data_type_set(snd_hdl,
}

