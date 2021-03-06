#include <stdio.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <list>
#include <ngl_dmx.h>
#include <ngl_video.h>
#include <dvbepg.h>
#include <satellite.h>
#include <utils.h>
#include <ngl_os.h>
#include <windows.h>

using namespace nglui;

class DTVUI:public testing::Test{
public:
   SEARCHNOTIFY notify;
public :
   virtual void SetUp(){
      nglDmxInit();
      nglAvInit();
      LoadSatelliteFromDB("satellites.xml");
   }
   virtual void TearDown(){
   }
};

#define CHANNEL_LIST_ITEM_HEIGHT 40
class ChannelItem:public ListView::ListItem{
public:
    SERVICELOCATOR svc;
public :   
    ChannelItem(const std::string&txt,const SERVICELOCATOR*loc):ListView::ListItem(txt){
        svc=*loc;
    }
    virtual void onGetSize(AbsListView&lv,int* w,int* h)override{
        if(h)*h=CHANNEL_LIST_ITEM_HEIGHT;
    }
};

#define MSG_CHANNEL 1234
#define IDC_LISTVIEW 12
class TVWindow:public Window{
public:
    std::vector<std::shared_ptr<ChannelItem>>channels;
public:
     TVWindow(int x,int y,int w,int h):Window(x,y,w,h){}
     virtual bool onMessage(DWORD msg,DWORD wp,ULONG lp){
        ListView*lv=(ListView*)findViewById(IDC_LISTVIEW);
        switch(msg){
        case MSG_CHANNEL:
            for(auto a:channels){
                lv->addItem(a);
            }channels.clear();
            lv->setIndex(0);
            return true;
        default:
            return Window::onMessage(msg,wp,lp);
        }
     }
};

static void TPFINISH_CBK(const TRANSPONDER*,int idx,int tpcount,void*userdata){
   TVWindow*w=(TVWindow*)userdata;
   w->sendMessage(MSG_CHANNEL,0,0);
}

static INT SVC_CBK(const SERVICELOCATOR*loc,const DVBService*svc,void*userdata)
{
   TVWindow*w=(TVWindow*)userdata;
   char sname[128];
   svc->getServiceName(sname,nullptr);
   w->channels.push_back(std::shared_ptr<ChannelItem>(new ChannelItem(sname,loc)));
   return 1;
}

TEST_F(DTVUI,Search){
   INT count=0;
   NGLTunerParam tp;
   tp.delivery_type=DELIVERY_S;
   tp.u.s.symbol_rate=27500;//27500;//26040;
   tp.u.s.polar=NGL_NIM_POLAR_HORIZONTAL;// NGL_NIM_POLAR_HORIZONTAL NGL_NIM_POLAR_VERTICAL;
   tp.u.s.frequency=3840000;

   memset(&notify,0,sizeof(SEARCHNOTIFY));
   Window*w=new TVWindow(100,100,800,600);
   ListView *lv=new ListView(300,500);
   w->setLayout(new LinearLayout());
   w->addChildView(new TextField("DTV SERACH 节目搜索",600,30));
   w->addChildView(lv)->setId(IDC_LISTVIEW);
   notify.SERVICE_CBK=SVC_CBK;
   notify.FINISHTP=TPFINISH_CBK;
   notify.userdata=w;
   DtvSearch(&tp,1,&notify);
   ASSERT_TRUE(1);
   nglSleep(10000);
}

static void onChannelItemSelect(AbsListView&lv,int index){
     ChannelItem*itm=(ChannelItem*)lv.getItem(index);
     if(itm)DtvPlay(&itm->svc,nullptr);
};

static INT SVC_CBK2(const SERVICELOCATOR*loc,const DVBService*svc,void*userdata)
{
   char servicename[128];
   ListView*lv=(ListView*)userdata;
   svc->getServiceName(servicename,nullptr);
   lv->addItem(new ChannelItem(servicename,loc));
   return 1;
}
 
TEST_F(DTVUI,ChannelList){
   Window*w=new Window(100,100,800,600);
   ListView *lv=new ListView(300,500);
   printf("ChannelList.lv=%p\r\n",lv);
   w->setLayout(new LinearLayout());
   w->addChildView(new TextField("DTV CHANNELLIST 节目列表",600,30));
   w->addChildView(lv)->setId(IDC_LISTVIEW);
   lv->setItemSelectListener(onChannelItemSelect);
   lv->setItemSelectListener([](AbsListView&lv,int index){
       ChannelItem*itm=(ChannelItem*)lv.getItem(index);
       if(itm)DtvPlay(&itm->svc,nullptr);
   });
   DtvEnumService(SVC_CBK2,lv);
   nglSleep(100000);
}
