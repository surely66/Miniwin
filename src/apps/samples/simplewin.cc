#include<windows.h>
#include<ngl_ir.h>
#include<ngl_log.h>
NGL_MODULE(SIMPLEWIN)

static bool onKey(int key){
    NGLOG_DEBUG("rcvkey %d",key);
    switch(key){
    case NGL_KEY_ESCAPE:exit(0);
    default:
    Window*w=new Window(100,100,800,600);
    w->setLayout(new LinearLayout());
    w->addChildView(new TextField("HelloWorld!",400,80));
    }
}

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Desktop*desktop=new Desktop();
    int count=0;
    Window*w=nullptr;
    desktop->setKeyListener(onKey);
    if(argc==1){//it's recomand that use C++11 lambda listener for simple listener function
#if __cplusplus>201700
       desktop->setKeyListener([&w](int key)->bool{
             NGLOG_DEBUG("rcvkey %d",key);
             switch(key){
             case NGL_KEY_ESCAPE:exit(0);
             default:
                 w=new Window(100,100,800,600);
                 w->setLayout(new LinearLayout());
                 w->addChildView(new TextField("HelloWorld!!",400,80));
                 break;
             }return true; 
          }
       );
       desktop->setMessageListener([&count,&w,&desktop](DWORD msg,DWORD wp,ULONG lp)->bool{
            if(msg!=1234)return false;
            if(w)w->sendMessage(View::WM_DESTROY,0,0);
            w=new Window(100,100,800,600);
            w->setLayout(new LinearLayout());
            w->addChildView(new TextField("HelloWorld!! "+std::to_string(count++),400,80)); 
            desktop->sendMessage(msg,wp,lp,500);
       });
#endif
    }else{//if you dont known C++11 lambda,used like following lines
       desktop->setKeyListener(onKey);
    }
    desktop->sendMessage(1234,0,0);
    return app.exec();
}
