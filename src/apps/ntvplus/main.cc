extern "C"{
#include <va_pvr.h>
}
#include <windows.h>
#include <ngl_ir.h>
#include <appmenus.h>
#include <dvbapp.h>
#include <dvbepg.h>
#include <satellite.h>
#include <diseqc.h>
#include <string>

using namespace ntvplus;

int main(int argc,const char*argv[]){
    DVBApp app(argc,argv);
    Desktop*desktop=new Desktop();
    DWORD pvr_hdl=0;
    app.setName("com.ntvplus.dvbs");

    app.setOpacity(app.getArgAsInt("alpha",255));
    app.getString("mainmenu",app.getArg("language","eng"));
    desktop->setKeyListener([&](int key)->bool{
         printf("rcv key:%x menu=%x\r\n",key,NGL_KEY_MENU);
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
         case NGL_KEY_ESCAPE:exit(0);
         default:return false;
         }
    });
    return app.exec();
}
