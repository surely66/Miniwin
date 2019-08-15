#include <stdio.h>
#include <sys/time.h>
#include <gtest/gtest.h>
extern "C"{
#include <ngl_os.h>
}

class OSMUTEX:public testing::Test{
   
   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(OSMUTEX,MUTEX_Create_1){
   NGLMutex mutex;
   ASSERT_TRUE(0==nglCreateMutex(&mutex));
   ASSERT_TRUE(0==nglDeleteMutex(mutex));
}

TEST_F(OSMUTEX,MUTEX_Create_2){
   NGLMutex mtx;
   ASSERT_FALSE(0==nglCreateMutex(NULL));
}

TEST_F(OSMUTEX,MUTEX_Delete_1){
   NGLMutex mtx;
   ASSERT_TRUE(0==nglCreateMutex(&mtx));
   ASSERT_TRUE(0==nglDeleteMutex(mtx));
}

TEST_F(OSMUTEX,MUTEX_Delete_Error_1){
   ASSERT_FALSE(0==nglDeleteMutex(NULL));
}

TEST_F(OSMUTEX,MUTEX_Lock_1){
   NGLMutex mtx;
   ASSERT_TRUE(0==nglCreateMutex(&mtx));
   ASSERT_TRUE(0==nglLockMutex(mtx));
   ASSERT_TRUE(0==nglDeleteMutex(mtx));
}

TEST_F(OSMUTEX,MUTEX_Lock_2){
   NGLMutex mtx;
   ASSERT_TRUE(0==nglCreateMutex(&mtx));
   ASSERT_TRUE(0==nglLockMutex(mtx));
   ASSERT_TRUE(0==nglLockMutex(mtx));
   ASSERT_TRUE(0==nglDeleteMutex(mtx));
}

TEST_F(OSMUTEX,MUTEX_Lock_Unlock_1){
   NGLMutex mtx;
   ASSERT_TRUE(0==nglCreateMutex(&mtx));
   ASSERT_TRUE(0==nglLockMutex(mtx));
   ASSERT_TRUE(0==nglUnlockMutex(mtx));
   ASSERT_TRUE(0==nglDeleteMutex(mtx));
}
