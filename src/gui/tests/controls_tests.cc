#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>

using namespace nglui;
#define ID_OK 10
#define ID_CANCEL 15
#define ID_LISTVIEW 20
#define ID_TIPINFO 30
class CONTROLS:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

class AutoWin{
private:
   Window*win;
public:
   AutoWin(Window*w){win=w;}
   ~AutoWin(){win->sendMessage(View::WM_DESTROY,0,0);}
};

class MyWindow:public Window{
public:
      MyWindow(int x,int y,int w,int h):Window(x,y,w,h){}
      virtual bool onMessage(DWORD msgid,DWORD wparam,ULONG lparam){
         std::string txt;
         Widget*tip;
         switch(msgid){
         case WM_CLICK:
               txt="You clicked view which it's id:"+std::to_string(wparam);
               Toast::makeText(txt,2000)->setPos(300,wparam*20);
               break;
         case WM_SELCHANGE:
               tip=(Widget*)findViewById(ID_TIPINFO);
               txt="onMessage: You Select Item:"+std::to_string(lparam)+" of view id:"+std::to_string(wparam);
               if(tip)
                  tip->setText(txt);
               break;
         default:return false;
         }
         return true;
      }  
};
static void onClick(View&v){
    std::string txt="You clicked:";
    txt+=((Widget&)v).getText();
    txt+="   id:"+std::to_string(v.getId());
    Toast::makeText(txt,2000)->setPos(200,v.getId()*20);
}

TEST_F(CONTROLS,Button){
   Window*w=new MyWindow(100,100,800,600);
   AutoWin aw(w);
   w->setLayout(new LinearLayout());
   Button*btn1=new Button("OK",100,30);
   Button*btn2=new Button("Cancel",100,30);
   btn1->setId(ID_OK);
   btn1->setClickListener(onClick); //it's same as following lambda segment
   btn1->setClickListener([](View&v){
       std::string txt="You clicked:";
       txt+=((Widget&)v).getText();
       txt+="   id:"+std::to_string(v.getId());
       Toast::makeText(txt,2000)->setPos(200,v.getId()*20);
   });
   //btn2->setOnClickListener(click);//it click listener is not set ,view's parent will recv WM_CLICK message
   w->addChildView(btn1);
   w->addChildView(btn2);
   btn2->setId(ID_CANCEL);
   nglSleep(10000);
}

TEST_F(CONTROLS,EditBox){
    Window*w=new MyWindow(100,100,800,600);
    w->setLayout(new LinearLayout());
    w->addChildView(new EditBox(200,40));
    EditBox *e=new EditBox(200,40);
    e->setEditMode(1);
    w->addChildView(e);
    nglSleep(30000);
}

TEST_F(CONTROLS,ProgressBar){
    Window*w=new Window(100,100,800,600);
    AutoWin aw(w);
    w->setLayout(new LinearLayout());
    w->addChildView(new ProgressBar(800,20))->setId(100);
    w->addChildView(new ProgressBar(30,200))->setId(101);
    for(int i=0;i<=100;i+=5){
       ((ProgressBar*)w->findViewById(100))->setProgress(i);
       ((ProgressBar*)w->findViewById(101))->setProgress(i);
       nglSleep(100);
    }
    nglSleep(1000);
}

static const char*texts[]={
   "图书管理系统项目介绍","Android 实现书籍翻页效果---完结篇_图文",
   "delphi图书管理信息系统课程设计报告","安卓入门5_ 快速ListView 2",
   "12节-Flutter","ListView横向列表使用","必须使用UTF8编码","华为开发者大会2019",
   "DevRun·选择不凡 ","华为云开发者沙龙2019-重庆站","2019年华为开发者大赛","人工智能的兴起逐渐","改变了我们的工作和学习方式",
   "在可以预见的未来","人工智能将会主导社会发展速度。","而深度学习平台的出现","势必将人工智能技术","推向一个成熟的发展阶段"
};

static void onItemSelect(AbsListView&lv,int index){
     std::string txt="You Select Item:"+std::to_string(index)+" of view id:"+std::to_string(lv.getId());
     Toast::makeText(txt,2000)->setPos(200,lv.getId()*20);
};

TEST_F(CONTROLS,ListView){
    Window*w=new MyWindow(100,100,800,620);
    //AutoWin aw(w);
    w->setLayout(new LinearLayout());
    w->addChildView(new ListView(400,500))->setId(ID_LISTVIEW);
    ListView*lv=(ListView*)w->addChildView(new ListView(300,500));
    lv->setId(22);

    Selector*ls=new Selector("CHOISE",600,28);
    ls->setId(23);ls->setLabelWidth(120);
    ls->setPopupRect(600,100,300,620);
    w->addChildView(ls);//3

    TextField*tt=new TextField("Tips:",600,30);
    tt->setId(ID_TIPINFO);
    w->addChildView(tt);//4
    int cnt=sizeof(texts)/sizeof(char*);
    for(int j=0;j<3;j++)
    for(int i=0;i<cnt;i++){
         ((ListView*)w->findViewById(ID_LISTVIEW))->addItem(new ListView::ListItem(texts[i]));
         lv->addItem(new ListView::ListItem(texts[i%cnt]));
         ls->addItem(new ListView::ListItem(texts[i%cnt]));
    }
    lv->setItemSelectListener(onItemSelect);
    lv->setItemSelectListener([](AbsListView&lv,int index){
         std::string txt="You Select Item:"+std::to_string(index)+" of view id:"+std::to_string(lv.getId());
         Toast::makeText(txt,2000)->setPos(200,lv.getId()*20); 
    });
    nglSleep(28000);
}

#include<channelepgview.h>
#include<ngl_timer.h>

TEST_F(CONTROLS,ChannelEPGView){
    Window*w=new MyWindow(100,100,800,620);
    ChannelEpgView*v=new ChannelEpgView(800,500);
    w->addChildView(v);
    ULONG start;
    for(int i=0;i<10;i++){
        nglGetTime(&start);
        ChannelBar*ch=new ChannelBar("channel:"+std::to_string(i));
        for(int j=0;j<20;j++){
            ULONG dur=30+(i*5+j)%30;dur*=60;
            ch->addEvent("",start,dur);start+=dur;
        }
        v->addItem(ch);
    }
    nglGetTime(&start);
    for(int i=0;i<60;i++){
       v->setStartTime(start);start+=300;
       v->invalidate(nullptr);
       nglSleep(100);
    }
    nglSleep(28000);
}

