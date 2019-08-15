#include<windows.h>
#include<ngl_ir.h>

static const char*texts[]={"Creates 中国智造"," the specified format and dimensions.",
            "Initially the surface contents"," are set to 0.","(Specifically, within each pixel,",
            " each color or alpha channel","belonging to format will be 0.","The contents","of bits within a pixel,",
            " but not belonging","必须使用UTF8编码 " };

class MWindow:public Window{
private:
   int dx,dy;
public:
   MWindow(int x,int y,int w,int h):Window(x,y,w,h){dx=dy=50;}
   virtual bool onMessage(DWORD msgid,DWORD wp,ULONG lp){
      if(View::WM_TIMER==msgid){
         sendMessage(msgid,wp,lp,lp);
         int x=getX();
         int y=getY();
         if(x+getWidth()+dx>1280)dx*=-1;
         if(y+getHeight()+dy>720)dy*=-1;
         if(x+dx<0)dx*=-1;
         if(y+dy<0)dy*=-1;
         setPos(x+dx,y+dy);
         return true;
      }
      return false;
   }
   void setDir(int x,int y){dx=x;dy=y;}
};


int main(int argc,const char*argv[]){
    App app(argc,argv);
    Desktop*desktop=new Desktop();
        MWindow*w3=new MWindow(200,300,480,320);
    MWindow*w1=new MWindow(100,100,480,320);
    MWindow*w2=new MWindow(300,200,480,320);
    w1->setLayout(new StackLayout());
    w2->setLayout(new LinearLayout());
    w1->setBgColor(0xFFFF0000);
    w2->setBgColor(0xFF00FF00);
    for(int i=0;i<4;i++){
      ListView *lv1=new ListView(400,200);
      ListView *lv2=new ListView(200,100);
      w1->addChildView(lv1);
      w2->addChildView(lv2);
      for(int j=0;j<8;j++){
           lv1->addItem(new ListView::ListItem(texts[j]));
           lv2->addItem(new ListView::ListItem(texts[j]));
      }
      lv1->setIndex(i);
      lv2->setIndex(i+i);
    }
    w1->sendMessage(View::WM_TIMER,0,100,100);w1->setDir(50,40);
    w2->sendMessage(View::WM_TIMER,0,100,80);w2->setDir(66,53);
    w3->sendMessage(View::WM_TIMER,0,100,85);w3->setDir(69,57);

    return app.exec();
}
