#include<windows.h>
#include<appmenus.h>
#include<ngl_log.h>
#include<ngl_timer.h>
#include <app.h>
#define IDC_CHANNELS 100
#define IDC_GUIDE    101
#define IDC_MEDIA    102
#define IDC_SETTINGS 103
#define IDC_ACCOUNT  104
#define IDC_DATE     200
#define IDC_TIME     201
NGL_MODULE(MAINMENU)

namespace ntvplus{

static void onButtonClick(View&v){
    NGLOG_DEBUG("onButtonClick %d",v.getId());
    switch(v.getId()){
    case IDC_CHANNELS:CreateChannelList();break;
    case IDC_GUIDE: CreateTVGuide();break;
    case IDC_MEDIA: 
            CreateMediaWindow();
            break;
    case IDC_SETTINGS:CreateSettingWindow();break;
    case IDC_ACCOUNT:
           break;
    default:break;
    }     
}

Window*CreateMainMenu(){
   const char* btnname[]={"channels","tvguide","multimedia","syssettings","account"};
   const char*imgname[]={
          "channel_uns.png","channel_sel.png",
          "tv_uns.png","tv_sel.png",
          "media_uns.png","media_sel.png",
          "setting_uns.png","setting_sel.png",
          "ca_uns.png","ca_sel.png"
   };
   NTVWindow*w=new NTVWindow(0,0,1280,720);
   w->initContent(NWS_TITLE|NWS_TOOLTIPS);
   w->setText("mainmenu");
   for(int i=0;i<sizeof(btnname)/sizeof(char*);i++){
       ImageButton *btn=new ImageButton(btnname[i],220,230);
       if(i==0)btn->setFlag(View::Attr::ATTR_FOCUSED);
       btn->setPos(70+i*230,300);
       btn->setBgColor(0xFF000000).setFgColor(0xFFFFFFFF).setFontSize(30);
       btn->setImage(imgname[2*i]);
       btn->setHotImage(imgname[2*i+1]);
       btn->setId(IDC_CHANNELS+i);
       btn->setClickListener(onButtonClick);
       w->addChildView(btn);      
   }
   w->addTipInfo("help_icon_4arrow.png","Navigation",400,160);
   w->addTipInfo("help_icon_ok.png","Select",-1,160);
   w->addTipInfo("help_icon_exit.png","Exit",-1,160);
   
   NGLOG_DEBUG("show mainmenu");
   w->show();
   return w;
}

}//namespace
