#include<windows.h>
int main(int argc,const char*argv[]){

    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    w->setLayout(new LinearLayout());
    w->addChildView(new TextField("HelloWorld",400,80));
    return app.exec();
}
