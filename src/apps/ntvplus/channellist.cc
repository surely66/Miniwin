#include <windows.h>
#include <ntvwindow.h>
#include <appmenus.h>
#include <ngl_types.h>
#include <dvbepg.h>
#include <ngl_log.h>
#include <ngl_panel.h>
#define IDC_CHANNELS 100
#define IDC_NAMEP 101
#define IDC_DESCP  102
#define IDC_NAMEF 103
#define IDC_DESCF  104
#define IDC_TPINFO 105
NGL_MODULE(CHANNELLIST)

namespace ntvplus{

class ChannelsWindow:public NTVWindow{
protected:
   ListView*chlst;
   TextField*namep,*descp;
   TextField*namef,*descf;
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
    tbfavs=CreateNTVToolBar(1280,38);

    tbfavs->setPos(0,70);
    loadGroups();
    addChildView(tbfavs);

    RefPtr<Gradient>pat=LinearGradient::create(0,0,w,h);
    pat->add_color_stop_rgba(.0,.2,.2,.2,.2);
    pat->add_color_stop_rgba(.5,1.,1.,1.,1.);
    pat->add_color_stop_rgba(1.,.2,.2,.2,.2);

    chlst=new ListView(400,520);
    chlst->setPos(50,110);
    chlst->setBgPattern(pat);
    chlst->setFgColor(getFgColor());
    chlst->setFlag(View::Attr::ATTR_SCROLL_VERT);
    chlst->setFlag(View::Attr::ATTR_BORDER);
    chlst->setItemPainter(ChannelPainterLCN);
    addChildView(chlst)->setId(IDC_CHANNELS);
    addChildView(new TextField("tpinfo",200,28))->setId(IDC_TPINFO).setPos(1000,630).setFontSize(20);
    chlst->setItemSelectListener([](AbsListView&lv,int index){
        ChannelItem*itm=(ChannelItem*)lv.getItem(index);
        //if(itm)DtvPlay(&itm->svc,nullptr);
        int lcn;char buf[8];
        DtvGetServiceItem(&itm->svc,SKI_LCN,&lcn);
        sprintf(buf,"%d",lcn);
        nglFPShowText(buf,4);
        lv.getParent()->sendMessage(1000,0,0,500);
        TextField*tpinfo=(TextField*)lv.getParent()->findViewById(IDC_TPINFO);
        TRANSPONDER tp; 
        int rc=DtvGetTPByService(&itm->svc,&tp);
        NGLOG_VERBOSE("DtvGetTPByService=%d tpinfo=%p",rc,tpinfo);
        tpinfo->setText(GetTPString(&tp)); 
    });
    if(favgroups.size())loadServices(favgroups[0]);
    namep=new TextField("NOW",400,36);
    addChildView(namep)->setId(IDC_NAMEP).setBgPattern(pat).setFgColor(0xFFFFFFFF).setPos(455,110);

    descp=new TextField(std::string(),800,220);
    descp->setAlignment(DT_LEFT|DT_TOP|DT_MULTILINE);
    addChildView(descp)->setPos(455,148).setId(IDC_DESCP).setBgPattern(pat);

    namef=new TextField("NEXT",400,36);
    addChildView(namef)->setId(IDC_NAMEF).setBgPattern(pat).setFgColor(0xFFFFFFFF).setPos(455,370);

    descf=new TextField(std::string(),800,220);
    descf->setAlignment(DT_LEFT|DT_TOP|DT_MULTILINE);
    addChildView(descf)->setPos(455,408).setId(IDC_DESCF).setBgPattern(pat);

    
    addTipInfo("help_icon_4arrow.png","Navigation",50,160);
    addTipInfo("help_icon_ok.png","Select",-1,160);
    addTipInfo("help_icon_exit.png","Exit",-1,260);
    addTipInfo("help_icon_red.png","Sort",-1,160);
    addTipInfo("help_icon_yellow.png","EditChannel",-1,160);

    setMessageListener([this](View&v,DWORD msg,DWORD wp,ULONG lp)->bool{
        if(msg==1000){
             DVBEvent pf[2];
             NGLOG_VERBOSE("Get Event P/F to ui");
             char name[256],des[256];
             ListView*lv=(ListView*)v.findViewById(IDC_CHANNELS);
             int idx=lv->getIndex();
             if(idx<0)return false;
             ChannelItem*itm=(ChannelItem*)lv->getItem(idx);
             int rc=DtvGetPFEvent(&itm->svc,pf);
             NGLOG_VERBOSE("DtvGetPFEvent=%d",rc); 
                       
             if(rc&1){pf[0].getShortName(name,des);namep->setText(name);descp->setText(des);}
             if(rc&2){pf[1].getShortName(name,des);namef->setText(name);descf->setText(des);}           
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
   case NGL_KEY_ENTER:
        {
            int idx=chlst->getIndex();
            ChannelItem*itm=(ChannelItem*)chlst->getItem(idx);
            if(itm)DtvPlay(&itm->svc,nullptr); 
        }break;
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
          NGLOG_VERBOSE("%x %s %d services",favid,name,FavGetServiceCount(favid));
     }
     tbfavs->setIndex(0);
     return count;
}

int ChannelsWindow::loadServices(UINT favid){
     char name[128];
     size_t count=FavGetServiceCount(favid);
     SERVICELOCATOR*svcs=new SERVICELOCATOR[count];
     FavGetGroupName(favid,name);
     NGLOG_DEBUG("%x[%s] has %d svc",favid,name,count);
     chlst->clearAllItems();
     SERVICELOCATOR cur;
     DtvGetCurrentService(&cur);
     for(size_t i=0;i<count;i++){
          SERVICELOCATOR svc;
          FavGetService(favid,&svc,i);
          const DVBService*info=DtvGetServiceInfo(&svc);
          if(NULL==info)continue;
          info->getServiceName(name);
          ChannelItem*ch=new ChannelItem(name,&svc,info->freeCAMode); 
          INT lcn;
          DtvGetServiceItem(&svc,SKI_LCN,&lcn);
          ch->setId(lcn);
          ch->isHD=ISHDVIDEO(info->serviceType);
          NGLOG_VERBOSE("    %d %d.%d.%d.%d:%s  %p hd=%d type=%d",i,svc.netid,svc.tsid,svc.sid,svc.tpid,name,info,ch->isHD,info->serviceType);
          chlst->addItem(ch);
          if(svc.sid==cur.sid&&svc.tsid==cur.tsid&&cur.netid==svc.netid)
             chlst->setIndex(i);
     }
     NGLOG_DEBUG("%d services loaded CUR:%d.%d.%d index=%d",chlst->getItemCount(),cur.netid,cur.tsid,cur.sid,chlst->getIndex());
     chlst->sort([](const ListView::ListItem&a,const ListView::ListItem&b)->int{
                            return a.getId()-b.getId()<0;
                       },false);
     delete svcs;
}

Window*CreateChannelList(){
    ChannelsWindow*w=new ChannelsWindow(0,0,1280,720);
    w->show();
    return w;
}
}//namespace
