extern "C"{
#include <va_pvr.h>
}
#include <windows.h>
#include <ngl_ir.h>
#include <ngl_video.h>
#include <appmenus.h>
#include <dvbapp.h>
#include <dvbepg.h>
#include <satellite.h>
#include <diseqc.h>
#include <string>
#include <ngl_log.h>
NGL_MODULE(MAIN)
using namespace ntvplus;

int main(int argc,const char*argv[]){
    DVBApp app(argc,argv);
    Desktop*desktop=new Desktop();
    DWORD pvr_hdl=0;
    app.setName("com.ntvplus.dvbs");

    app.setOpacity(app.getArgAsInt("alpha",255));
    app.getString("mainmenu",app.getArg("language","eng"));
    desktop->setKeyListener([&](int key)->bool{
         switch(key){
         case NGL_KEY_MENU:CreateMainMenu();return true;
         case NGL_KEY_ENTER:CreateChannelList();return true;
         case NGL_KEY_UP:
         case NGL_KEY_DOWN:CreateChannelPF();return true;
         case NGL_KEY_EPG: CreateTVGuide();return true;
         case NGL_KEY_F4:
                 return true;
         case NGL_KEY_F5:
                 return true;
         case NGL_KEY_AUDIO:
             ShowAudioSelector(ST_AUDIO,4000);return true;
         case NGL_KEY_POWER:exit(0);
         case NGL_KEY_VOL_INC:
         case NGL_KEY_VOL_DEC:
              {
                  INT vol=nglSndGetColume(0);
                  vol+=key==NGL_KEY_VOL_INC?5:-5;
                  nglSndSetVolume(0,vol);
                  ShowVolumeWindow(2000);
              }break;
         default:return false;
         }
    });
    return app.exec();
}
