#include <stdio.h>
#include <sys/time.h>
#include <gtest/gtest.h>
#include <ngl_os.h>

class OSSem:public testing::Test{
   
   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(OSSem,SEM_Create_1){
   NGLSemaphore sem;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,0));
   ASSERT_TRUE(0==nglDeleteSemaphore(sem));
}

TEST_F(OSSem,SEM_Create_2){
   NGLSemaphore sem;
   ASSERT_TRUE(0!=nglCreateSemaphore(NULL,0));
}

TEST_F(OSSem,SEM_Delete_1){
   NGLSemaphore sem;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,0));
   ASSERT_TRUE(0==nglDeleteSemaphore(sem));
}

TEST_F(OSSem,SEM_Delete_Error_1){
   ASSERT_FALSE(0==nglDeleteSemaphore(0));
}

TEST_F(OSSem,SEM_Acquire_1){
   NGLSemaphore sem;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,1));
   ASSERT_TRUE(0==nglAcquireSemaphore(sem,-1));
}

TEST_F(OSSem,SEM_Acquire_2){
   NGLSemaphore sem;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,2));
   ASSERT_TRUE(0==nglAcquireSemaphore(sem,-1));
   ASSERT_TRUE(0==nglAcquireSemaphore(sem,-1));
   ASSERT_TRUE(0==nglDeleteSemaphore(sem));
}

static void SemPostProc(void*p){
   printf("SemPostProc sem=%p",p);
   sleep(5);
   nglReleaseSemaphore((DWORD)p);
   printf("nglReleaseSemaphore %p",p);
}
TEST_F(OSSem,SEM_Acquire_FOREVER){
   NGLSemaphore sem; 
   DWORD tid;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,0));
   nglCreateThread(&tid,0,0,SemPostProc,(void*)sem);
   ASSERT_TRUE(0==nglAcquireSemaphore(sem,-1));
}

TEST_F(OSSem,SEM_Acquire_Timeout_1){
   NGLSemaphore sem;
   struct timeval tv1,tv2;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,1));
   gettimeofday(&tv1,NULL);
   ASSERT_TRUE(0==nglAcquireSemaphore(sem,1000));
   gettimeofday(&tv2,NULL);
   unsigned long long dur=tv2.tv_sec*1000l+tv2.tv_usec/1000-tv1.tv_sec*1000l-tv1.tv_usec/1000;
   ASSERT_FALSE(dur<1010&&dur>990);
   ASSERT_TRUE(0==nglDeleteSemaphore(sem));
}
TEST_F(OSSem,SEM_Acquire_Timeout_2){
   NGLSemaphore sem;
   struct timeval tv1,tv2;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,0));
   gettimeofday(&tv1,NULL);
   ASSERT_FALSE(0==nglAcquireSemaphore(sem,1000));
   gettimeofday(&tv2,NULL);
   unsigned long long dur=tv2.tv_sec*1000l+tv2.tv_usec/1000-tv1.tv_sec*1000l-tv1.tv_usec/1000;
   ASSERT_TRUE(dur<1010&&dur>990);
   ASSERT_TRUE(0==nglDeleteSemaphore(sem));
}


TEST_F(OSSem,SEM_Release_1){
   NGLSemaphore sem;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,1));
   ASSERT_TRUE(0==nglReleaseSemaphore(sem));
   ASSERT_TRUE(0==nglDeleteSemaphore(sem));
}

TEST_F(OSSem,SEM_Release_2){
   NGLSemaphore sem;
   ASSERT_TRUE(0==nglCreateSemaphore(&sem,0));
   ASSERT_TRUE(0==nglReleaseSemaphore(sem));
   ASSERT_TRUE(0==nglAcquireSemaphore(sem,-1));
   ASSERT_TRUE(0==nglDeleteSemaphore(sem));
}
