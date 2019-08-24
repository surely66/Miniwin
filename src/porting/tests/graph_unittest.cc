#include<ngl_graph.h>
#include<ngl_os.h>
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
   nglFillRect(hwsurface,&r1,0x00000);
   nglFlip(hwsurface);

   nglCreateSurface(&swsurface,800,600,0,0);
   NGLRect rf={0,0,800,600};
   nglFillRect(swsurface,&rf,0xFF444444);
   NGLRect r={100,50,640,480};
   nglFillRect(swsurface,&r,0x0);
   for(int i=-1;i<10;i++){
       blitflag=(i==-1)?0:(1<<i);
       for(int j=0;j<13;j++){
           porterduff=j;printf("blitflags=%x:%x\r\n",blitflag,porterduff);
           nglBlit(hwsurface,swsurface,NULL,NULL);
           nglFlip(hwsurface);
           nglSleep(200);
       }
   }
   nglDestroySurface(swsurface);
   nglDestroySurface(hwsurface);
}
