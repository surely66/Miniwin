#include <stdio.h>
#include <gtest/gtest.h>
#include <ngl_os.h>

class OSMEM:public testing::Test{
   public :
   void*p;
   virtual void SetUp(){}
   virtual void TearDown(){}
};

TEST_F(OSMEM,Malloc_1){
   ASSERT_EQ(NULL,nglMalloc(0));
}

TEST_F(OSMEM,Malloc_2){
   p=nglMalloc(100);
   ASSERT_NE(p,(void*)NULL);
   nglFree(p);
}

TEST_F(OSMEM,Realloc_1){
   p=nglMalloc(100);
   ASSERT_TRUE(p);
   ASSERT_TRUE(p=nglRealloc(p,200));
   nglFree(p);
}

TEST_F(OSMEM,Realloc_2){
   p=nglRealloc(NULL,200);
   ASSERT_TRUE(p);
   nglFree(p);
}

