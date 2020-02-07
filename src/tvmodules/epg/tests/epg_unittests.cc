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
      nglTunerInit();
      nglDmxInit();
      nglAvInit();
      LoadSatelliteFromDB("satellites.xml");
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
static void FilterCBK(HANDLE dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData){
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
        printf("TS[%2d] freq=%d symbol=%d pos=%d direction=%d modulation=%d fec=%d\r\n",i,tp.u.s.frequency,tp.u.s.symbol_rate,tp.u.s.position,
             tp.u.s.direction,tp.u.s.modulation,tp.u.s.fec);
   }
}
TEST_F(EPGTest,BAT){
   BYTE mask[8],match[8];
   HANDLE flt=nglAllocateSectionFilter(1,PID_BAT,FilterCBK,NULL,NGL_DMX_SECTION);
   mask[0]=0xFF;
   match[0]=TBID_BAT;
   nglSetSectionFilterParameters(flt,mask,match,1);
   nglStartSectionFilter(flt);
   nglSleep(10000);
   DtvInitLCN((LCNMODE)(LCN_FROM_BAT|LCN_FROM_USER),1000);
}

TEST_F(EPGTest,NIT){
   BYTE mask[8],match[8];
   HANDLE flt=nglAllocateSectionFilter(1,PID_NIT,FilterCBK,NULL,NGL_DMX_SECTION);
   mask[0]=0xFF;
   match[0]=TBID_NIT;
   nglSetSectionFilterParameters(flt,mask,match,1);
   nglStartSectionFilter(flt);
   nglSleep(10000);
}

TEST_F(EPGTest,TEST){
   INT count=0;
   TRANSPONDER tp;
   SEARCHNOTIFY notify;
   tp.delivery_type=DELIVERY_S;
   tp.u.s.tpid=4;
   tp.u.s.symbol_rate=27500;//27500;//26040;
   tp.u.s.polar=NGL_NIM_POLAR_HORIZONTAL;// NGL_NIM_POLAR_HORIZONTAL NGL_NIM_POLAR_VERTICAL;
   tp.u.s.frequency=10750*1000;
   nglTunerSetLNB(0,1);
   nglTunerSet22K(0,1);
   ASSERT_EQ(NGL_OK,nglTunerLock(0,&tp));
   diseqc_set_diseqc10(0,TUNER_DISEQC10_PORT_A,TUNER_POL_HORIZONTAL,TUNER_TONE_22K_ON);//TUNER_POL_VERTICAL/HORIZONTAL
   ConfigureTransponder(&tp);
   memset(&notify,0,sizeof(SEARCHNOTIFY));
   //DtvSearch(&tp,1,&notify);
   ASSERT_TRUE(1);
   nglSleep(8000);
   DtvEnumService(SVC_CBK,&svcs);
   for(auto s:svcs){
      DtvPlay(&s,nullptr);
      nglSleep(10000);
   }
   ASSERT_TRUE(svcs.size()>0);
}
