#include<ngl_graph.h>
#include <gtest/gtest.h>

class GRAPH:public testing::Test{
   public :
   virtual void SetUp(){
     nglGraphInit();
   }
   virtual void TearDown(){
   }
};

TEST_F(GRAPH,Graph_GetScreen){
   UINT w,h;
   nglGetScreenSize(&w,&h);
   ASSERT_TRUE(w>0&&h>0);
}

TEST_F(GRAPH,CreateSurface){
   DWORD surface=0;
   UINT width,height;
   nglGetScreenSize(&width,&height);
   nglCreateSurface(&surface,width,height,0,1);
   NGLRect r={100,100,400,400};
   nglFillRect(surface,&r,0xFFFF0000);
   nglFlip(surface);
   ASSERT_TRUE(surface!=0);
   sleep(2);
   nglDestroySurface(surface);
}

TEST_F(GRAPH,Surface_Draw){
   DWORD surface=0;
   UINT width,height;
   UINT*buffer;
   UINT pitch;
   nglGetScreenSize(&width,&height);
   nglCreateSurface(&surface,width,height,0,1);
   ASSERT_TRUE(surface!=0);
   nglLockSurface(surface,(void**)&buffer,&pitch);
   for(int i=0;i<width*400;i++)     buffer[i]=0xFFFFFFFF;
   nglUnlockSurface(surface);
   NGLRect r={100,100,400,400};
   nglFillRect(surface,&r,0xFFFF00FF);
   nglFlip(surface);
   sleep(3);
   nglDestroySurface(surface);

}

TEST_F(GRAPH,Blit){
   DWORD hwsurface=0,swsurface;
   nglCreateSurface(&hwsurface,1280,720,0,1);
   NGLRect r1={0,0,1280,720};
   nglFillRect(hwsurface,&r1,0xFFFFFFFF);
   nglCreateSurface(&swsurface,800,600,0,0);
   NGLRect r={0,0,800,600};
   nglFillRect(swsurface,&r,0xFF0000FF);
  
   nglBlit(hwsurface,swsurface,NULL,&r);
   nglFlip(hwsurface);
   sleep(3);
   nglDestroySurface(swsurface);
   nglDestroySurface(hwsurface);
}
