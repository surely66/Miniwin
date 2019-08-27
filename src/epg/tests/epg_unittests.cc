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
#include <diseqc.h>

class EPGTest:public testing::Test{
public:
   std::vector<SERVICELOCATOR> svcs;
   public :
   virtual void SetUp(){
      std::vector<int>ll;
      int todo=8;
      for(int i=0;i<10;i++)ll.push_back(i);
      std::vector<int>::iterator itr=std::find(ll.begin(),ll.end(),todo);
      printf("std::find=%d\r\n",itr==ll.end());//?*itr:-1));
      for(itr=ll.begin();itr!=ll.end();itr++){
          if(*itr==8)printf("found 8\r\n"); 
      }
      if(itr==ll.end())printf("not found\r\n");
      nglTunerInit();
      nglDmxInit();
      nglAvInit();
      LoadSatelliteFromDB("satellites.xml");
      /*const char *gb="你好,GB2UTF测试";
      char gbs[32]={0x13};
      char utf[32];
      strcpy(gbs+1,gb);
      int rc=ToUtf8(gbs,strlen(gb)+1,utf);
      printf("ToUtf8=%d utf=%s\r\n",rc,utf);*/
   }
   virtual void TearDown(){
   }
};

static INT SVC_CBK(const SERVICELOCATOR*loc,const DVBService*svc,void*userdata)
{
   char servicename[128],providername[128];
   svc->getServiceName(servicename,providername);
   printf("\tservice[%04d] %x.%x.%x %s:%s\r\n",(*(int*)userdata),loc->netid,loc->tsid,loc->sid,servicename,providername);
   std::vector<SERVICELOCATOR>*svcs=(std::vector<SERVICELOCATOR>*)userdata;
   svcs->push_back(*loc);
   return 1;
}

static SERVICELOCATOR svcs[64];
static USHORT lcn[64];
static int cnt=0;
static void FilterCBK(DWORD dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData){
   BAT b(pBuffer);
   NIT n(pBuffer);
   char name[128];
   DVBStream tss[32];
   if(b.tableId()==TBID_BAT)AddBATSection(b,NULL);
   int tsc=n.getStreams(tss,false);
   int rc=n.getName(name,NULL);
   printf("name:%s  rc=%d\r\n",name,rc);
   for(int i=0;i<tsc;i++){
        NGLTunerParam tp;
        tss[i].getDelivery(&tp);
        printf("TS[%2d] freq=%d symbol=%d pos=%d direction=%d\r\n",i,tp.u.s.frequency,tp.u.s.symbol_rate,tp.u.s.position,
             tp.u.s.direction);
   }
}
TEST_F(EPGTest,BAT){
   BYTE mask[8],match[8];
   DWORD flt=nglAllocateSectionFilter(1,PID_BAT,FilterCBK,NULL,NGL_DMX_SECTION);
   mask[0]=0xFF;
   match[0]=TBID_BAT;
   nglSetSectionFilterParameters(flt,1,mask,match);
   nglStartSectionFilter(flt);
   nglSleep(10000);
   DtvInitLCN((LCNMODE)(LCN_FROM_BAT|LCN_FROM_USER),1000);
}

TEST_F(EPGTest,NIT){
   BYTE mask[8],match[8];
   DWORD flt=nglAllocateSectionFilter(1,PID_NIT,FilterCBK,NULL,NGL_DMX_SECTION);
   mask[0]=0xFF;
   match[0]=TBID_NIT;
   nglSetSectionFilterParameters(flt,1,mask,match);
   nglStartSectionFilter(flt);
   nglSleep(10000);
}

TEST_F(EPGTest,TEST){
   INT count=0;
   TRANSPONDER tp;
   SEARCHNOTIFY notify;
   tp.delivery_type=DELIVERY_S;
   tp.u.s.tpid=1;
   tp.u.s.symbol_rate=27500;//27500;//26040;
   tp.u.s.polar=NGL_NIM_POLAR_HORIZONTAL;// NGL_NIM_POLAR_HORIZONTAL NGL_NIM_POLAR_VERTICAL;
   tp.u.s.frequency=3840*1000;
   nglTunerSetLNB(0,1);
   nglTunerSet22K(0,1);
   ASSERT_EQ(NGL_OK,nglTunerLock(0,&tp));
   diseqc_set_diseqc10(0,TUNER_DISEQC10_PORT_A,TUNER_POL_HORIZONTAL,TUNER_TONE_22K_ON);//TUNER_POL_VERTICAL/HORIZONTAL
   ConfigureTransponder(&tp);
   memset(&notify,0,sizeof(SEARCHNOTIFY));
   DtvSearch(&tp,1,&notify);
   ASSERT_TRUE(1);
   nglSleep(8000);
   DtvEnumService(SVC_CBK,&svcs);
   for(auto s:svcs){
      DtvPlay(&s,nullptr);
      nglSleep(10000);
   }
   ASSERT_TRUE(svcs.size()>0);
}
