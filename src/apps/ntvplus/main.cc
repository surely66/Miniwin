#include <windows.h>
#include <ngl_ir.h>
#include <appmenus.h>
#include <dvbapp.h>
#include <dvbepg.h>
#include <satellite.h>
#include <diseqc.h>

using namespace ntvplus;
int main(int argc,const char*argv[]){
    DVBApp app(argc,argv);
    Desktop*desktop=new Desktop();
    app.setName("com.ntvplus.dvbs");
    app.setOpacity(0xAA);
    desktop->setKeyListener([](int key)->bool{
         printf("rcv key:%x menu=%x\r\n",key,NGL_KEY_MENU);
         switch(key){
         case NGL_KEY_MENU:CreateMainMenu();return true;
         case NGL_KEY_ENTER:CreateChannelList();return true;
         case NGL_KEY_UP:
         case NGL_KEY_DOWN:CreateChannelPF();return true;
         case NGL_KEY_ESCAPE:exit(0);
         default:return false;
         }
    });
    return app.exec();
}
