#include<appmenus.h>
#include<ntvwindow.h>
#include<ngl_log.h>

NGL_MODULE(CHANNELBAR)

namespace ntvplus{

static INT SVC_CBK2(const SERVICELOCATOR*loc,const DVBService*svc,void*userdata){
   ListView*lv=(ListView*)userdata;
   SERVICELOCATOR cur;
   char servicename[128];
   svc->getServiceName(servicename);
   DtvGetCurrentService(&cur);
   lv->addItem(new ChannelItem(servicename,loc,svc->freeCAMode));
   if(cur==*loc){
        lv->setIndex(lv->getItemCount()-1);
        NGLOG_DEBUG("last played service =%d.%d.%d index=%d",loc->netid,loc->tsid,loc->sid,lv->getItemCount()-1);
   }
   return 1;
}
#define MSG_DATE_TIME 1010
class ChannelBar:public NTVWindow{
protected:
   bool present;
   Selector *chlst;
   ProgressBar*media_progress;
   TextField*event_name;
   TextField*event_des;
   TextField*datetime ;
public:
   ChannelBar(int x,int y,int w,int h);
   virtual bool onKeyRelease(KeyEvent&k)override;
   void getEvents();
   virtual bool onMessage(DWORD msgid,DWORD wp,ULONG lp)override{
        if(msgid==MSG_DATE_TIME){
             char buf[64];
             time_t now=time(NULL);
             struct tm *tnow=localtime(&now);
             sprintf(buf,"%02d/%02d/%d %02d:%02d:%02d",tnow->tm_mon,tnow->tm_mday,tnow->tm_year+1900,tnow->tm_hour,tnow->tm_min,tnow->tm_sec);
             datetime->setText(buf);
             getEvents();
             sendMessage(MSG_DATE_TIME,wp,lp,1000);
             return true;
        }else return NTVWindow::onMessage(msgid,wp,lp);
   }
};
void ChannelBar::getEvents(){
    SERVICELOCATOR loc;
    DVBEvent pf[2],*p=nullptr;
    char name[256],desc[256];
    ChannelItem*itm=(ChannelItem*)chlst->getItem(chlst->getIndex());
    if(itm) loc=itm->svc;
    else DtvGetCurrentService(&loc);
    int rc=DtvGetPFEvent(&loc,pf);
    if( (rc&1) && present ){
         time_t now=time(NULL);
         p=pf;
         media_progress->setProgress((now-pf->start_time)*100/pf->duration);
    }
    if( (rc&2) &&(present==false) ) p=pf+1;
    if(p){ 
        p->getShortName(name,desc);
        event_name->setText(name);
        event_des->setText(desc);
    }
}

ChannelBar::ChannelBar(int x,int y,int w,int h):NTVWindow(x,y,w,h){
   initContent(NWS_SIGNAL);
   chlst=new Selector(std::string(),495,46);
   chlst->showArrows(Selector::SHOW_NEVER);
   chlst->setFontSize(40);
   chlst->clearFlag(Attr::ATTR_FOCUSABLE);
   chlst->setBgColor(0xFF000000).setFgColor(0xFFFFFFFF);
   addChildView(chlst);
   chlst->setItemSelectListener([](AbsListView&lv,int index){
        ChannelItem*itm=(ChannelItem*)lv.getItem(index);
        if(itm)DtvPlay(&itm->svc,nullptr);
    });

   event_name=new TextField(std::string(),495,46);
   event_name->setFontSize(28);
   event_name->setPos(500,0);
   addChildView(event_name)->setBgColor(0xFF000000).setFgColor(0xFFFFFFFF);

   event_des=new TextField(std::string(),880,180);
   event_des->setPos(200,51);
   addChildView(event_des)->setBgColor(0xFF222222).setFgColor(0xFFFFFFFF);

   datetime=new TextField(std::string(),280,46);
   datetime->setFontSize(28);
   datetime->setPos(1000,0);
   addChildView(datetime)->setBgColor(0xFF000000).setFgColor(0xFFFFFFFF);

   sig_strength->setSize(400,10);
   sig_strength->setPos(20,51);
   sig_quality->setSize(400,10);
   sig_quality->setPos(20,66);
   
   media_progress=new NTVProgressBar(w,3);
   media_progress->setPos(0,46);
   media_progress->setProgress(37);
   addChildView(media_progress);
   DtvEnumService(SVC_CBK2,chlst);
   sendMessage(MSG_DATE_TIME,0,0,1000);
   present=true;   
}

bool ChannelBar::onKeyRelease(KeyEvent&k){
   switch(k.getKeyCode()){
   case NGL_KEY_UP:  
           k.setKeyCode(NGL_KEY_LEFT) ;
           return chlst->onKeyRelease(k);
   case NGL_KEY_DOWN:
           k.setKeyCode(NGL_KEY_RIGHT);
           return chlst->onKeyRelease(k);
   case NGL_KEY_LEFT:
   case NGL_KEY_RIGHT:
         present=!present;
         getEvents();
         return true;
   default:return NTVWindow::onKeyRelease(k);
   }
}

Window*CreateChannelPF(){
   Window*w=new ChannelBar(0,480,1280,240);
   w->show();
   return w;
}
}//namespace
