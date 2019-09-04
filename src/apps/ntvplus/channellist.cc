#include <windows.h>
#include <ntvwindow.h>
#include <appmenus.h>
#include <ngl_types.h>
#include <dvbepg.h>
#include <ngl_log.h>
#include <ngl_panel.h>
#define IDC_CHANNELS 100
#define IDC_EVENTP 101
#define IDC_EVENTF 102
#define IDC_TPINFO 103
NGL_MODULE(CHANNELLIST)

namespace ntvplus{

class ChannelsWindow:public NTVWindow{
protected:
   ListView*chlst;
   TextField*eventp;
   TextField*eventf;
   ToolBar*tbfavs;
   std::vector<unsigned int>favgroups;
public:
   ChannelsWindow(int x,int y,int w,int h);
   virtual bool onKeyRelease(KeyEvent&k)override;
   int loadGroups();
   int loadServices(UINT favid);
};

ChannelsWindow::ChannelsWindow(int x,int y,int w,int h):NTVWindow(x,y,w,h){
    initContent(NWS_TITLE|NWS_TOOLTIPS);
    setText("ChannelList");
    tbfavs=new NTVToolBar(1280,38);
    tbfavs->setPos(0,70);
    loadGroups();
    addChildView(tbfavs);

    chlst=new ListView(400,520);
    chlst->setPos(50,110);
    chlst->setBgColor(getBgColor());
    chlst->setFgColor(getFgColor());
    chlst->setFlag(View::Attr::ATTR_SCROLL_VERT);
    chlst->setFlag(View::Attr::ATTR_BORDER);
    chlst->setItemPainter(ChannelPainterLCN);
    addChildView(chlst)->setId(IDC_CHANNELS);
    addChildView(new TextField("tpinfo",200,28))->setId(IDC_TPINFO).setPos(1000,630).setFontSize(20);
    chlst->setItemSelectListener([](AbsListView&lv,int index){
        ChannelItem*itm=(ChannelItem*)lv.getItem(index);
        if(itm)DtvPlay(&itm->svc,nullptr);
        int lcn;char buf[8];
        DtvGetServiceItem(&itm->svc,SKI_LCN,&lcn);
        sprintf(buf,"%d",lcn);
        nglFPShowText(buf,4);
        lv.getParent()->sendMessage(1000,0,0,500);
        TextField*tpinfo=(TextField*)lv.getParent()->findViewById(IDC_TPINFO);
        TRANSPONDER tp; 
        int rc=DtvGetTPByService(&itm->svc,&tp);
        NGLOG_DEBUG("DtvGetTPByService=%d tpinfo=%p",rc,tpinfo);
        tpinfo->setText(GetTPString(&tp)); 
    });
    if(favgroups.size())loadServices(favgroups[0]);

    addChildView(new TextField("NOW",100,36))->setBgColor(0xFF222222).setFgColor(0xFFFFFFFF).setPos(455,110);

    eventp=new TextField(std::string(),800,220);
    eventp->setAlignment(DT_LEFT|DT_VCENTER|DT_MULTILINE);
    addChildView(eventp)->setPos(455,148).setId(IDC_EVENTP).setBgColor(0xFF444444);

    addChildView(new TextField("NEXT",100,36))->setBgColor(0xFF222222).setFgColor(0xFFFFFFFF).setPos(455,370);

    eventf=new TextField(std::string(),800,220);
    eventf->setAlignment(DT_CENTER|DT_TOP|DT_MULTILINE);
    addChildView(eventf)->setPos(455,408).setId(IDC_EVENTF).setBgColor(0xFF222222);

    
    addTipInfo("help_icon_4arrow.png","Navigation",50,160);
    addTipInfo("help_icon_ok.png","Select",-1,160);
    addTipInfo("help_icon_exit.png","Exit",-1,260);
    addTipInfo("help_icon_red.png","Sort",-1,160);
    addTipInfo("help_icon_yellow.png","EditChannel",-1,160);

    setMessageListener([](View&v,DWORD msg,DWORD wp,ULONG lp)->bool{
        if(msg==1000){
             DVBEvent p,f;
             NGLOG_DEBUG("Get Event P/F to ui");
             char name[256],des[256];
             ListView*lv=(ListView*)v.findViewById(IDC_CHANNELS);
             int idx=lv->getIndex();
             if(idx<0)return false;
             ChannelItem*itm=(ChannelItem*)lv->getItem(idx);
             int rc=DtvGetPFEvent(&itm->svc,&p,&f);
             NGLOG_DEBUG("DtvGetPFEvent=%d",rc); 
             TextField*tvp=(TextField*)v.findViewById(IDC_EVENTP);
             TextField*tvf=(TextField*)v.findViewById(IDC_EVENTF);
                       
             if(rc&1){p.getShortName(name,des);tvp->setText(name);}
             if(rc&2){f.getShortName(name,des);tvf->setText(name);}           
        }
    });    
}

bool ChannelsWindow::onKeyRelease(KeyEvent&k){
   int cnt=tbfavs->getButtonCount();
   int idx;
   switch(k.getKeyCode()){
   case NGL_KEY_LEFT :
          idx=(tbfavs->getIndex()+cnt-1)%cnt;
          tbfavs->setIndex(idx);
          loadServices(favgroups[idx]);
          return true;
   case NGL_KEY_RIGHT:
          idx=(tbfavs->getIndex()+1)%cnt;
          tbfavs->setIndex(idx); 
          loadServices(favgroups[idx]);
          return true;
   case KEY_YELLOW:CreateChannelEditWindow();return true;
   default:return NTVWindow::onKeyRelease(k);
   }
}

int ChannelsWindow::loadGroups(){
     int count=FavGetGroupCount();
     for(int i=0;i<count;i++){
          char name[64];
          UINT favid;
          FavGetGroupInfo(i,&favid,name);
          favgroups.push_back(favid);
          tbfavs->addButton(name,(i==0?80:-1),200);
          NGLOG_VERBOSE("%x %s",favid,name);
     }
     tbfavs->setIndex(0);
     return count;
}

int ChannelsWindow::loadServices(UINT favid){
     size_t count=FavGetServiceCount(favid);
     SERVICELOCATOR*svcs=new SERVICELOCATOR[count];
     NGLOG_DEBUG("%x %d's service",favid,count);
     chlst->clearAllItems();
     SERVICELOCATOR cur;
     DtvGetCurrentService(&cur);
     for(size_t i=0;i<count;i++){
          SERVICELOCATOR svc;
          FavGetService(favid,&svc,i);
          const DVBService*info=DtvGetServiceInfo(&svc);
          char servicename[128];
          info->getServiceName(servicename);
          ChannelItem*ch=new ChannelItem(servicename,&svc,info->freeCAMode); 
          INT lcn;
          DtvGetServiceItem(&svc,SKI_LCN,&lcn);
          ch->setValue(lcn);
          ch->isHD=ISHDVIDEO(info->serviceType);
          NGLOG_VERBOSE("    %d:%s  %p hd=%d type=%d",i,servicename,info,ch->isHD,info->serviceType);
          chlst->addItem(ch);
          if(svc.sid==cur.sid&&svc.tsid==cur.tsid&&cur.netid==svc.netid)
             chlst->setIndex(i);
     }
     delete svcs;
}

Window*CreateChannelList(){
    ChannelsWindow*w=new ChannelsWindow(0,0,1280,720);
    w->show();
    return w;
}
}//namespace
