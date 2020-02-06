#include <stdio.h>
#include <gtest/gtest.h>
#include <ngl_os.h>
#include <ngl_ir.h>
#include <fcntl.h>
#include <linux/input.h>
class IRINPUT:public testing::Test{
   public :
   virtual void SetUp(){
       nglIrInit();
   }
   virtual void TearDown(){}
};

TEST_F(IRINPUT,Open){
   HANDLE hdl=nglIrOpen(0,NULL);
   ASSERT_TRUE(hdl);
   nglIrClose(hdl);
}
TEST_F(IRINPUT,GetKey){
   HANDLE hdl=nglIrOpen(0,NULL);
   int i=0;
   while(i++<100){
      NGLKEYINFO key;
      nglIrGetKey(hdl,&key,500);
   }
   nglIrClose(hdl);
}
