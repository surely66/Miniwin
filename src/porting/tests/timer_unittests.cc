#include <gtest/gtest.h>
#include <ngl_timer.h>
#include <time.h>

class TIMER:public testing::Test{
   public :
   void*p;
   virtual void SetUp(){}
   virtual void TearDown(){}
};

TEST(TIMER,SetTime){
   NGL_TIME t;
   nglGetTime(&t);
   t+=100*3600*24*365;
   struct  tm tt;
   tt.tm_hour=18;
   tt.tm_min=18;
   tt.tm_sec=18;
   tt.tm_year=18;
   tt.tm_mon=5; //月份（从一月开始，0代表一月） - 取值区间为[0,11]
   tt.tm_mday=18;
   t=mktime(&tt);
   nglSetTime(&t);
}
