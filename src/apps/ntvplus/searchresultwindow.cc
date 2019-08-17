#include<appmenus.h>
#include <dvbapp.h>
#include <dvbepg.h>
#include <satellite.h>
#include <diseqc.h>
#include <ngl_log.h>
NGL_MODULE(SEARCHING)

namespace ntvplus{

static INT SVC_CBK(const SERVICELOCATOR*loc,const DVBService*svc,void*userdata);
static void NEW_TP(const TRANSPONDER*,int idx,int tp_count,void*userdata);
static void FINISHEDTP(const TRANSPONDER*,int idx,int tp_count,void*userdata);
class SearchResultWindow:public NTVWindow{
protected:
    ListView*left;
    ListView*right;
    ProgressBar*progress;
    TextField*percent;
    SEARCHNOTIFY notify;
public:
    std::vector<TRANSPONDER>transponders;
public:
    SearchResultWindow(int x,int y,int w,int h);
    void Search(std::vector<TRANSPONDER>tps,int mode){
         transponders=tps;
         notify.SERVICE_CBK=SVC_CBK;
         notify.FINISHTP=FINISHEDTP;
         notify.NEWTP=NEW_TP;
         notify.userdata=this;
         NGLOG_DEBUG("search %d tp",transponders.size());
         DtvSearch(transponders.data(),transponders.size(),&notify);
    }
    void onReceivedService(const SERVICELOCATOR*loc,const DVBService*svc){
         char servicename[128],providername[128];
         svc->getServiceName(servicename,providername);
         switch(svc->serviceType){
         case SVC_VIDEO:left->addItem(new ChannelItem(servicename,loc));break;
         case SVC_AUDIO:right->addItem(new ChannelItem(servicename,loc));break;
         }
         NGLOG_DEBUG("rcv service %d.%d.%d %d[%s]",loc->netid,loc->tsid,loc->sid,svc->serviceType,servicename);
    }
    void onFinishedTP(const TRANSPONDER*tp,int idx,int tp_count){
         progress->setProgress(idx*100/tp_count);
         percent->setText(std::to_string(idx*100/tp_count)+"%");
         if(nullptr==tp){
              ChannelItem*itm=(ChannelItem*)left->getItem(0);
              if(itm)DtvPlay(&itm->svc,nullptr);
              DtvSaveProgramsData("dvb_programs.dat");
         }
    }
};

SearchResultWindow::SearchResultWindow(int x,int y,int w,int h)
:NTVWindow(x,y,w,h){
     initContent(NWS_TITLE|NWS_SIGNAL|NWS_TOOLTIPS); 
     left =new ListView(560,400);
     right=new ListView(560,400);
     left->setFlag(Attr::ATTR_BORDER);
     right->setFlag(Attr::ATTR_BORDER);
     left->setPos(60,70);
     right->setPos(650,70);
     left->setItemPainter(ChannelPainter);
     right->setItemPainter(ChannelPainter);
 
     addChildView(left);
     addChildView(right);
     left->setBgColor(getBgColor());
     left->setFgColor(getFgColor());
     right->setBgColor(getBgColor());
     right->setFgColor(getFgColor());
      
     sig_strength->setSize(300,10);
     sig_strength->setPos(50,580);
     sig_quality->setSize(300,10);
     sig_quality->setPos(50,600);    
     addChildView(sig_strength);
     addChildView(sig_quality);

     percent=new TextField("0%",200,120);
     percent->setPos(1000,520);
     addChildView(percent)->setBgColor(0xFF000000).setFgColor(0xFFFFFFFF).setFontSize(80);

     progress=new ProgressBar(1000,10);
     progress->setPos(50,620);progress->setProgress(35);
     addChildView(progress);

     memset(&notify,0,sizeof(SEARCHNOTIFY));
}

Window*CreateSearchResultWindow(std::vector<TRANSPONDER>tps,int mode){
     SearchResultWindow*w=new SearchResultWindow(0,0,1280,720);
     w->setText("Search"); 
     w->Search(tps,mode);
     w->addTipInfo("help_icon_4arrow.png","Navigation",50,160);
     w->addTipInfo("help_icon_ok.png","Select",-1,160);
     w->addTipInfo("help_icon_back.png","Back",-1,160);
     w->addTipInfo("help_icon_exit.png","Exit",-1,260);
     w->addTipInfo("help_icon_blue.png","",-1,160);
     w->addTipInfo("help_icon_red.png","",-1,160);
     w->addTipInfo("help_icon_yellow.png","",-1,160);

     w->show();
     return w;
}

static INT SVC_CBK(const SERVICELOCATOR*loc,const DVBService*svc,void*userdata){
    SearchResultWindow*w=(SearchResultWindow*)userdata;
    w->onReceivedService(loc,svc);
    return 1;
}
static void NEW_TP(const TRANSPONDER*tp,int,int,void*userdata){
    SearchResultWindow*w=(SearchResultWindow*)userdata;
    NGLOG_DEBUG("Searching TP freq:%d",tp->u.s.frequency);
}

static void FINISHEDTP(const TRANSPONDER*tp,int idx,int cnt,void*userdata){
    SearchResultWindow*w=(SearchResultWindow*)userdata;
    w->onFinishedTP(tp,idx,cnt);
    if(tp)
        NGLOG_DEBUG("TP freq:%d Search Finished",tp->u.s.frequency);
    else
        NGLOG_DEBUG("Search Finished");
}
}//namespace