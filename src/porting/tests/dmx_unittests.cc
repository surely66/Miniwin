#include <stdio.h>
#include <gtest/gtest.h>
#include <ngl_os.h>
#include <ngl_dmx.h>
#include <ngl_tuner.h>
#define MAX_PROGRAM 32

class DMX:public testing::Test{
   public :
   DWORD flt;
   DWORD eventHandle;
   BYTE*data;
   unsigned int program_count;
   unsigned short pmtpids[MAX_PROGRAM+MAX_PROGRAM];
   virtual void SetUp(){
      NGLTunerParam tp;//TRANSPONDER tp;
      tp.delivery_type=DELIVERY_S;
      tp.u.s.tpid=4;
      tp.u.s.symbol_rate=27500;//27500;//26040;
      tp.u.s.polar=NGL_NIM_POLAR_HORIZONTAL;// NGL_NIM_POLAR_HORIZONTAL NGL_NIM_POLAR_VERTICAL;
      //tp.u.s.frequency=11600*1000;//
      tp.u.s.frequency=10750*1000;
      nglTunerInit();
      //nglTunerSetLNB(0,1);    nglTunerSet22K(0,1);
      //nglTunerLock(0,&tp);
      nglDmxInit();
      flt=0;
      data=(BYTE*)nglMalloc(4096);
      eventHandle=nglCreateEvent(0,1);
      program_count=0;
   }
   virtual void TearDown(){
      if(0!=flt)
          nglFreeSectionFilter(flt);
      nglFree(data);
   }
   int GetPMTPids(BYTE*pmt){
      int seclen=((pmt[1]&0x0F)<<8)|pmt[2];
      int cnt=0;
      BYTE*p=pmt+8;
      unsigned short* pids=pmtpids;
      for(;p<pmt+seclen-1;p+=4){
         pids[0]=(p[0]<<8)|p[1]; //serviceid
         pids[1]=((p[2]&0x1F)<<8)|p[3]; //pmtpid
         if(pids[1]==0x10)continue;
         printf("serviceid=0x%x/%d pmt_pid=0x%x/%d\r\n",pids[0],pids[0],pids[1],pids[1]);
         pids+=2;cnt++;
      }program_count=cnt;
      return cnt;
  }
};

static void SectionCBK(DWORD dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData)
{
   printf("SectionCBK flt=0x%x data=%p 0x%02x\n",dwVaFilterHandle,pBuffer,pBuffer[0]);  
}

TEST_F(DMX,AllocFilter_1){
   flt=nglAllocateSectionFilter(0,0x10,SectionCBK,NULL,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   ASSERT_EQ(nglFreeSectionFilter(flt),NGL_OK);
   flt=nglAllocateSectionFilter(0,0x10,SectionCBK,NULL,NGL_DMX_TS);
   ASSERT_FALSE(0==flt);
   ASSERT_EQ(nglFreeSectionFilter(flt),NGL_OK);
}

TEST_F(DMX,AllocFilter_2){
   flt=nglAllocateSectionFilter(0,0x10,NULL,NULL,NGL_DMX_SECTION);
   ASSERT_TRUE(0==flt);
}

TEST_F(DMX,StartFilter_1){
   flt=nglAllocateSectionFilter(0,0x10,SectionCBK,NULL,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   ASSERT_TRUE(0==nglStartSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,StopFilter_1){
   flt=nglAllocateSectionFilter(0,0x10,SectionCBK,NULL,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   ASSERT_EQ(0,nglFreeSectionFilter(flt));
}

TEST_F(DMX,StartFilter_ERROR_1){
   ASSERT_FALSE(0==nglStartSectionFilter(0));
}

TEST_F(DMX,StopFilter_ERROR_1){
   ASSERT_FALSE(0==nglStopSectionFilter(0));
}

TEST_F(DMX,SetFilterParam_1){
   BYTE mask[8],value[8];
   flt=nglAllocateSectionFilter(0,0x10,SectionCBK,NULL,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,SetFilterParam_Error_1){
   BYTE mask[8],value[8];
   flt=nglAllocateSectionFilter(0,0x10,SectionCBK,NULL,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,NULL,NULL,0));
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,NULL,NULL,1));
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,mask,NULL,1));
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,mask,value,0));
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,NULL,value,0));
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,NULL,value,0));
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,SetFilterParam_Error_2){
   BYTE mask[8],value[8];
   flt=nglAllocateSectionFilter(0,0x10,SectionCBK,NULL,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,mask,value,9));
   nglFreeSectionFilter(flt);
}

static void FilterCBK(DWORD dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData){
   int i;
   void**params=(void**)pUserData;
   printf("FilterCBK flt=%p  params[0]=%p\r\n",dwVaFilterHandle,params[0]);
   memcpy(params[0],pBuffer,uiBufferLength);
   nglSetEvent((DWORD)params[1]);
}

TEST_F(DMX,Filter_Section_1){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   flt=nglAllocateSectionFilter(0,0x00,FilterCBK,params,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   mask[0]=0xFF;value[0]=0x00;//for PAT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_TRUE(0==nglWaitEvent(eventHandle,2000));
   ASSERT_EQ(data[0],0);
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,Filter_TS_1){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   flt=nglAllocateSectionFilter(0,0x00,FilterCBK,params,NGL_DMX_TS);
   ASSERT_FALSE(0==flt);
   mask[0]=0xFF;value[0]=0x00;//for PAT
   //ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_TRUE(0==nglWaitEvent(eventHandle,2000));
   ASSERT_EQ(data[0],0x47);
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,Filter_Section_DMX_1){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   flt=nglAllocateSectionFilter(1,0x00,FilterCBK,params,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   mask[0]=0xFF;value[0]=0x00;//for PAT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_TRUE(0==nglWaitEvent(eventHandle,2000));
   ASSERT_EQ(data[0],0);
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,Filter_Data_2){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   for(int i=0;i<3;i++){
     flt=nglAllocateSectionFilter(0,0x11,FilterCBK,params,NGL_DMX_SECTION);
     ASSERT_FALSE(0==flt);
     mask[0]=0xFF;value[0]=0x42;//for SDT
     ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
     ASSERT_EQ(0,nglStartSectionFilter(flt));
     ASSERT_TRUE(0==nglWaitEvent(eventHandle,2000));
     ASSERT_EQ(data[0],0x42);
     ASSERT_TRUE(0==nglStopSectionFilter(flt));
     nglFreeSectionFilter(flt);
   }
}

TEST_F(DMX,Filter_Data_PAT_PMT){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   flt=nglAllocateSectionFilter(0,0,FilterCBK,params,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   mask[0]=0xFF;value[0]=0x0;//for PAT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_TRUE(0==nglWaitEvent(eventHandle,2000));
   ASSERT_EQ(data[0],0x00);
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   GetPMTPids(data);
   ASSERT_TRUE(program_count);
   nglFreeSectionFilter(flt);
   for(int i=0;i<program_count;i++){
       int masklen=1;
       mask[0]=mask[1]=mask[2]=mask[3]=mask[4]=0xFF;
       value[0]=0x02;
#if 1
       mask[1]=mask[2]=0; value[1]=value[2]=0;
       value[3]=pmtpids[i*2]>>8;
       value[4]=pmtpids[i*2]&0xFF;
       masklen=5;
#else
       value[1]=pmtpids[i*2]>>8;
       value[2]=pmtpids[i*2]&0xFF;
       masklen=3;
#endif 
       printf("masklen=%d \r\n",masklen);
       flt=nglAllocateSectionFilter(0,pmtpids[i*2+1],FilterCBK,params,NGL_DMX_SECTION);
       nglSetSectionFilterParameters(flt,mask,value,masklen);
       nglStartSectionFilter(flt);
       ASSERT_TRUE(0==nglWaitEvent(eventHandle,2000));
       ASSERT_EQ(data[0],2);
       ASSERT_EQ(data[3]<<8|data[4],pmtpids[i*2]);
       nglStopSectionFilter(flt);
       nglFreeSectionFilter(flt);
   }
}

TEST_F(DMX,Filter_Data_DMX1_2){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   flt=nglAllocateSectionFilter(1,0x11,FilterCBK,params,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   mask[0]=0xFF;value[0]=0x42;//for SDT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_TRUE(0==nglWaitEvent(eventHandle,2000));
   ASSERT_EQ(data[0],0x42);
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,Filter_Data_DMX1_3){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   flt=nglAllocateSectionFilter(1,0x400,FilterCBK,params,NGL_DMX_SECTION);
   ASSERT_FALSE(0==flt);
   mask[0]=0xFF;value[0]=0x02;//for PMT
   mask[1]=mask[2]=0x00;
   mask[3]=0xFF;value[3]=0x04;//program_number=0x401
   mask[4]=0xFF;value[4]=0x01;
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,3));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_TRUE(0==nglWaitEvent(eventHandle,2000));
   printf("exttableid=0x%02x%02x\r\n",data[3],data[4]);
   ASSERT_TRUE(data[3]==0x04 && data[4]==0x01);
   ASSERT_TRUE(0==nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}
