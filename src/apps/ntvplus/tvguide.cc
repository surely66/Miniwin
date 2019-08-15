#include <windows.h>
#include <appmenus.h>
#include <ngl_log.h>
#include <dvbepg.h>
#include <ngl_video.h>
#include <channelepgview.h>
#define CHANNEL_LIST_ITEM_HEIGHT 42

NGL_MODULE(TVGUIDE)

namespace ntvplus{

class TVChannel:public nglui::ChannelBar{
public:
   SERVICELOCATOR svc;
   int update=0;
public:
   TVChannel(const std::string&txt,const SERVICELOCATOR*svc_):ChannelBar(txt){
       svc=*svc_;
       update=0;
   }
};
static INT SVC_CBK2(const SERVICELOCATOR*loc,const DVBService*svc,void*userdata)
{
   char servicename[128];
   svc->getServiceName(servicename);   
   ChannelEpgView*lv=(ChannelEpgView*)userdata;
   lv->addItem(new TVChannel(servicename,loc));
   SERVICELOCATOR cur;
   DtvGetCurrentService(&cur);
   if(cur==*loc)lv->setIndex(lv->getItemCount()-1);
   return 1;
}
#define MSG_EPGS_UPDATE 1010
class TVWindow:public NTVWindow{
public:
   ChannelEpgView *lv;
   time_t tlast;
public:
   TVWindow(int x,int y,int w,int h):NTVWindow(x,y,w,h){
       lv=new ChannelEpgView(1200,340);
       lv->setPos(40,310);
       lv->setChannelNameWidth(320);
       tlast=0;
       addChildView(lv);
       sendMessage(MSG_EPGS_UPDATE,0,0,1000);
   }
   ~TVWindow(){
       nglAvSetVideoWindow(0,NULL,NULL);
   }
   virtual void onEITS(const SERVICELOCATOR*svc)override;
   virtual bool onKeyRelease(KeyEvent&k)override;
   virtual bool onMessage(DWORD msg,DWORD wp,ULONG lp)override;
};

void TVWindow::onEITS(const SERVICELOCATOR*svc){
    for(int i=0;i<lv->getItemCount();i++){
        TVChannel*ch=(TVChannel*)lv->getItem(i);
        if(ch->svc==*svc){
             ch->update++;
             break;
        }
    }
}

bool TVWindow::onMessage(DWORD msg,DWORD wp,ULONG lp){
    if(msg!=MSG_EPGS_UPDATE)
        return NTVWindow::onMessage(msg,wp,lp);
    for(int i=0;i<lv->getItemCount();i++){
        TVChannel*ch=(TVChannel*)lv->getItem(i);
        if(ch->update){
             char sname[256],des[256];
             ch->update==0;
             ch->clearEvents();
             std::vector<DVBEvent>evts;
             DtvGetEvents(&ch->svc,evts);
             for(auto e:evts){
                 e.getShortName(sname,des);
                 ch->addEvent(sname,e.start_time,e.duration);
             }
        }
    }
    sendMessage(MSG_EPGS_UPDATE,0,0,2000);
    return true;
}

bool TVWindow::onKeyRelease(KeyEvent&k){
    ULONG tt=lv->getStartTime();
    switch(k.getKeyCode()){
    case NGL_KEY_LEFT: lv->setStartTime(tt-7200);return true;
    case NGL_KEY_RIGHT:lv->setStartTime(tt+7200);return true;
    default:return NTVWindow::onKeyRelease(k);
    }
}

Window*CreateTVGuide(){
    TVWindow*w=new TVWindow(0,0,1280,720);
    w->initContent(NWS_TITLE|NWS_TOOLTIPS);
    w->setText("TV Guide");
    View*vv=new TextField(std::string(),320,240);
    vv->setFlag(View::Attr::ATTR_BORDER);
    vv->setPos(40,70);
    vv->setBgColor(0x0000);
    w->addChildView(vv); 
    NGLRect rcv={40,70,320,240};
    nglAvSetVideoWindow(0,NULL,&rcv);
    GroupView*g=new GroupView(880,240);
    g->setPos(361,70);
    g->setLayout(new LinearLayout());
    TextField*prg_title=new TextField("ProgramTitle",870,38);
    prg_title->setFontSize(35);
    TextField*prg_time=new TextField("Program time&duration",870,30);
    TextField*prg_dsc=new TextField("Program description",870,150);
    prg_title->setFgColor(0xFFFFFFFF);
    prg_time->setFgColor(0xFFFFFFFF);
    prg_dsc->setFgColor(0xFFFFFFFF);
    g->addChildView(prg_title);
    g->addChildView(prg_time);
    g->addChildView(prg_dsc);

    w->addChildView(g)->setBgColor(0xFF222222);
    
    w->addTipInfo("help_icon_4arrow.png","Navigation",50,160);
    w->addTipInfo("help_icon_ok.png","Select",-1,160);
    w->addTipInfo("help_icon_back.png","Back",-1,160);
    w->addTipInfo("help_icon_exit.png","Exit",-1,160);
    w->addTipInfo("help_icon_blue.png","",-1,160);
    w->addTipInfo("help_icon_red.png","",-1,160);
    w->addTipInfo("help_icon_yellow.png","",-1,160);
    
    w->lv->setItemSelectListener([](AbsListView&lv,int index){
       TVChannel*itm=(TVChannel*)lv.getItem(index);
       if(itm)DtvPlay(&itm->svc,nullptr);
    });
    DtvEnumService(SVC_CBK2,w->lv);
    w->show();
    return w; 
}

}//namespace
