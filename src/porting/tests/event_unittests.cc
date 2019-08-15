#include <stdio.h>
#include <sys/time.h>
#include <gtest/gtest.h>
#include <ngl_os.h>

class OSEvent:public testing::Test{
   public :
   struct timeval tv1,tv2;
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
   unsigned long long duration(){
       return tv2.tv_sec*1000l+tv2.tv_usec/1000-tv1.tv_sec*1000l-tv1.tv_usec/1000;
   }
};

TEST_F(OSEvent,Create_Auto_1){
   DWORD hdl=nglCreateEvent(0,1);//init as FALSE, auto reset
   gettimeofday(&tv1,NULL);
   ASSERT_TRUE(nglWaitEvent(hdl,1000)==NGL_ERROR);
   gettimeofday(&tv2,NULL);
   ASSERT_TRUE(duration()<1010&&duration()>990);
   ASSERT_TRUE(nglDestroyEvent(hdl)==NGL_OK);
}

TEST_F(OSEvent,Create_Auto_2){
   DWORD hdl=nglCreateEvent(1,1);//init as TRUE, auto reset
   gettimeofday(&tv1,NULL);
   ASSERT_TRUE(nglWaitEvent(hdl,1000)==NGL_OK);
   ASSERT_TRUE(nglWaitEvent(hdl,1000)==NGL_OK);
   gettimeofday(&tv2,NULL);
   ASSERT_TRUE(duration()<10);
   ASSERT_TRUE(nglDestroyEvent(hdl)==NGL_OK);
}

TEST_F(OSEvent,Create_Manual_1){
   DWORD hdl=nglCreateEvent(1,0);
   gettimeofday(&tv1,NULL);
   ASSERT_TRUE(nglWaitEvent(hdl,1000)==NGL_OK);
   gettimeofday(&tv2,NULL);
   ASSERT_TRUE(duration()<10);

   gettimeofday(&tv1,NULL);
   ASSERT_TRUE(nglWaitEvent(hdl,1000)==NGL_ERROR);
   gettimeofday(&tv2,NULL);
   ASSERT_TRUE(duration()<1010&&duration()>990);
   ASSERT_TRUE(nglDestroyEvent(hdl)==NGL_OK);
}

TEST_F(OSEvent,Create_Manual_2){
   DWORD hdl=nglCreateEvent(0,0);
   gettimeofday(&tv1,NULL);
   ASSERT_TRUE(nglWaitEvent(hdl,1000)==NGL_ERROR);
   gettimeofday(&tv2,NULL);
   ASSERT_TRUE(duration()<1010&&duration()>990);
   ASSERT_TRUE(nglWaitEvent(hdl,1000)==NGL_ERROR);
   gettimeofday(&tv2,NULL);
    ASSERT_TRUE(duration()<20010&&duration()>1990); 
   ASSERT_TRUE(nglDestroyEvent(hdl)==NGL_OK);
}


