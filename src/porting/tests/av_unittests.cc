#include <stdio.h>
#include <gtest/gtest.h>
#include <ngl_os.h>
#include <ngl_dmx.h>
#include <ngl_video.h>
#include <ngl_tuner.h>
class AV:public testing::Test{
   public :
   virtual void SetUp(){
     nglTunerInit();
     nglDmxInit();
     nglAvInit();
   }
   virtual void TearDown(){}
};

TEST_F(AV,Play){
   nglAvPlay(0,513,DECV_MPEG,660,DECA_MPEG1,8190);
   nglSleep(10000);
}

