#include <stdio.h>
#include <gtest/gtest.h>
#include <ngl_os.h>
#include <ngl_tuner.h>
#include <ngl_dmx.h>
#include <diseqc.h>

static void TunningCBK(INT tuneridx,INT lockedState,void*param){
    DWORD *params=(DWORD*)param;
    *((INT*)params[0])=lockedState;
    printf("lockedState=%d\r\n");
}
static void SectionCBK(DWORD dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData)
{
   printf("SectionCBK flt=0x%x data=%p 0x%02x\n",dwVaFilterHandle,pBuffer,pBuffer[0]);
   nglSetEvent((DWORD)pUserData);
}


class TUNER:public testing::Test{
   public :
   DWORD eventHandle;
   INT lockstate;
   DWORD params[2];
   DWORD flt;
   BYTE mask[8],value[8];
   virtual void SetUp(){
      nglTunerInit();
      nglDmxInit();
      lockstate=0;
      eventHandle=nglCreateEvent(0,0);
      params[0]=(DWORD)&lockstate;
      params[1]=eventHandle;
      nglTunerRegisteCBK(0,TunningCBK,params);
   }

   DWORD createFilter(){
      flt=nglAllocateSectionFilter(0,0/*patpid*/,SectionCBK,(void*)eventHandle,NGL_DMX_SECTION);
      mask[0]=0xFF;value[0]=0x00;//for PAT
      nglSetSectionFilterParameters(flt,1,mask,value); 
      nglStartSectionFilter(flt);
      return flt;
   }
   virtual void TearDown(){
      nglTunerUnRegisteCBK(0,TunningCBK);
   }
};

TEST_F(TUNER,LNB){
   //TUNER_Polarity 水平时为1，垂直时为2 off 时为0
   ASSERT_TRUE(nglTunerSetLNB(0,0)==NGL_OK);
   ASSERT_TRUE(nglTunerSetLNB(0,1)==NGL_OK);
   ASSERT_TRUE(nglTunerSetLNB(0,2)==NGL_OK);
}

TEST_F(TUNER,22K){
   ASSERT_TRUE(nglTunerSet22K(0,0)==NGL_OK);//22k off
   ASSERT_TRUE(nglTunerSet22K(0,1)==NGL_OK);//22k on
}

TEST_F(TUNER,Tunning_1){
   NGLTunerParam tp;
   tp.delivery_type=DELIVERY_S;
   tp.u.s.symbol_rate=27500;//27500;//26040;
   tp.u.s.polar=NGL_NIM_POLAR_HORIZONTAL;// NGL_NIM_POLAR_HORIZONTAL NGL_NIM_POLAR_VERTICAL;
   tp.u.s.frequency=3840*1000;
   printf("flt=%p\r\n",flt); 

   nglTunerSetLNB(0,1);//1--HORZ 2--VERT
   nglTunerSet22K(0,1);
   ASSERT_EQ(NGL_OK,nglTunerLock(0,&tp));
   diseqc_set_diseqc10(0,TUNER_DISEQC10_PORT_A,TUNER_POL_VERTICAL,TUNER_TONE_22K_ON);//TUNER_POL_VERTICAL/HORIZONTAL
   flt=createFilter();
   ASSERT_TRUE(NGL_OK==nglWaitEvent(eventHandle,5000));
   nglFreeSectionFilter(flt);
   ASSERT_EQ(1,lockstate);
}
TEST_F(TUNER,Tunning_Err_1){
   NGLTunerParam tp;
   tp.delivery_type=DELIVERY_S;
   tp.u.s.symbol_rate=27500;//27500;//26040;
   tp.u.s.polar=NGL_NIM_POLAR_VERTICAL;// NGL_NIM_POLAR_HORIZONTAL NGL_NIM_POLAR_VERTICAL;
   tp.u.s.frequency=38400*1000;
   ASSERT_EQ(NGL_ERROR,nglTunerLock(0,&tp));
   flt=createFilter();
   ASSERT_TRUE(NGL_OK!=nglWaitEvent(eventHandle,5000));
   nglFreeSectionFilter(flt);
   printf("\tfreq=%d locakstate=%d \r\n",tp.u.s.frequency,lockstate);
   ASSERT_EQ(0,lockstate);
}

