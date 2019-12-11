#include <stdio.h>
#include <sys/time.h>
#include <gtest/gtest.h>
extern "C"{
#include <ngl_os.h>
#include <ngl_msgq.h>
}

class OSMsgQ:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
   unsigned long long gettime(){
       struct timeval tv;
       gettimeofday(&tv,NULL);
       return tv.tv_sec*1000+tv.tv_usec/1000;
   }
};

TEST_F(OSMsgQ,Create_1){
   DWORD q=nglMsgQCreate(10,10);
   ASSERT_FALSE(0==q);
   ASSERT_TRUE(0==nglMsgQDestroy(q));
}

TEST_F(OSMsgQ,Create_Error_1){
   DWORD q=nglMsgQCreate(0,10);
   ASSERT_TRUE(0==q);
}

TEST_F(OSMsgQ,Create_Error_2){
   DWORD q=nglMsgQCreate(10,0);
   ASSERT_TRUE(0==q);
}

static void MsgQProc(void*p){
   int msg=3201;
   sleep(5);
   nglMsgQSend((DWORD)p,&msg,sizeof(int),-1);
}

TEST_F(OSMsgQ,Recv_1){
   int msg=1023;
   DWORD tid;
   DWORD q=nglMsgQCreate(10,sizeof(int));
   ASSERT_FALSE(0==q);
   ASSERT_TRUE(0==nglMsgQSend(q,&msg,sizeof(int),-1));
   msg=0;
   ASSERT_TRUE(0==nglMsgQReceive(q,&msg,sizeof(int),-1)); 
   ASSERT_TRUE(1023==msg);
   nglCreateThread(&tid,0,0,MsgQProc,(void*)q);
   ASSERT_TRUE(0==nglMsgQReceive(q,&msg,sizeof(int),-1));
   ASSERT_TRUE(3201==msg);
   ASSERT_TRUE(0==nglMsgQDestroy(q));
}


TEST_F(OSMsgQ,Recv_Timeout_1){
   int msg=1023;
   unsigned long long tv1,tv2;
   DWORD q=nglMsgQCreate(10,sizeof(int));
   ASSERT_FALSE(0==q);
   tv1=gettime();
   ASSERT_FALSE(0==nglMsgQReceive(q,&msg,sizeof(int),1000));
   tv2=gettime();
   ASSERT_TRUE(tv2-tv1<1010&&tv2-tv1>990);
   ASSERT_TRUE(0==nglMsgQDestroy(q));
}

TEST_F(OSMsgQ,Send_Timeout_1){
   int msg=1023;
   unsigned long long tv1,tv2;
   DWORD q=nglMsgQCreate(2,sizeof(int));
   ASSERT_FALSE(0==q);
   tv1=gettime();
   ASSERT_TRUE(0==nglMsgQSend(q,&msg,sizeof(int),1000));
   ASSERT_TRUE(0==nglMsgQSend(q,&msg,sizeof(int),1000));
   ASSERT_FALSE(0==nglMsgQSend(q,&msg,sizeof(int),1000));
   tv2=gettime();
   ASSERT_TRUE(tv2-tv1<1010&&tv2-tv1>990);
   ASSERT_TRUE(0==nglMsgQDestroy(q));
}

TEST_F(OSMsgQ,Send_Recv_1){
   int i,msgs[]={1,2,3,4,5,6,7,8};
   DWORD q=nglMsgQCreate(sizeof(msgs)/sizeof(int),sizeof(int));
   for(i=0;i<sizeof(msgs)/sizeof(int);i++)
       ASSERT_TRUE(0==nglMsgQSend(q,&msgs[i],sizeof(int),1000));
   for(i=0;i<sizeof(msgs)/sizeof(int);i++){
      int msg;
      ASSERT_TRUE(0==nglMsgQReceive(q,&msg,sizeof(int),1000));
      ASSERT_EQ(msg,i+1);
   }
}
static void SendProc(void*p){
   DWORD *dp=(DWORD*)p;
   DWORD msg,q=dp[0];
   int i=1;
   while(dp[i]){
      ASSERT_TRUE(0==nglMsgQSend(q,&dp[i],sizeof(DWORD),1000));
      i++;
      usleep(dp[1]);
   }
}
static void RecvProc(void*p){
   DWORD *dp=(DWORD*)p;
   DWORD msg,q=dp[0];
   int i=1;
   while(dp[i]){
      ASSERT_TRUE(0==nglMsgQReceive(q,&msg,sizeof(DWORD),1000));
      ASSERT_EQ(msg,dp[i]);
      i++;
      usleep(dp[2]);
   }
}

TEST_F(OSMsgQ,Send_Recv_2){//slow recv & fast send
   DWORD tid_snd,tid_rcv;
   DWORD i,msgs[]={0,100,200,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,0};
   DWORD q=nglMsgQCreate(6,sizeof(DWORD));
   msgs[0]=q;
   nglCreateThread(&tid_snd,0,0,SendProc,(void*)msgs);
   nglCreateThread(&tid_rcv,0,0,RecvProc,(void*)msgs);
   sleep(2);
}

TEST_F(OSMsgQ,Send_Recv_3){//fast recv & slow send
   DWORD tid_snd,tid_rcv;
   DWORD i,msgs[]={0,200,100,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,0};
   DWORD q=nglMsgQCreate(6,sizeof(DWORD));
   msgs[0]=q;
   nglCreateThread(&tid_snd,0,0,SendProc,(void*)msgs);
   nglCreateThread(&tid_rcv,0,0,RecvProc,(void*)msgs);
   sleep(2);
}
