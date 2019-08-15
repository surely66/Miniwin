#include<appmenus.h>
#include<ntvwindow.h>
#include<ngl_mediaplayer.h>
namespace ntvplus{

#define MSG_CURPOS 1010
class ControlWindow:public NTVWindow{
protected:
   std::vector<std::string>files;
   int cur_index;
   DWORD player;
   View*videovv;
   ProgressBar*progress;
public:
   ControlWindow(int x,int y,int w ,int h);
   ~ControlWindow(){
       if(player)nglMPClose(player);
   }
   void play(const std::string&url){
        if(player){
            nglMPStop(player);
            nglMPClose(player);
            player=0;
        }
        player=nglMPOpen(url.c_str());
        nglMPPlay(player);
   }
   virtual bool onMessage(DWORD msg,DWORD wp,ULONG lp)override{
        if( (msg==MSG_CURPOS) && player){
            UINT pos,duration;
            int rc=nglMPGetTime(player,&pos,&duration);
            if(rc==NGL_OK&&duration>0){
                progress->setProgress(pos);
                progress->setMax(duration);
            }
            return  true;
        }
        return NTVWindow::onMessage(msg,wp,lp);
   }
};

ControlWindow::ControlWindow(int x,int y,int w ,int h)
  :NTVWindow(x,y,w,h){
    initContent(NWS_TITLE|NWS_TOOLTIPS);
    
    videovv=new TextField(std::string(),1280,600);
    videovv->setFlag(View::Attr::ATTR_BORDER);
    videovv->setPos(0,70);
    videovv->setBgColor(0x0000).setFgColor(0xFFFF0000);
    addChildView(videovv);
  
    progress=new NTVProgressBar(1280,8);
    addChildView(progress)->setPos(0,660);
    player=0;
    sendMessage(MSG_CURPOS,0,0,1000);
}

Window*CreatePlayerCtrlWindow(const std::string&url){
    ControlWindow*w=new ControlWindow(0,0,1280,720);
    w->setText("PlayCtrl");
    w->show();
    w->play(url);
    return w;
}


}//namespace
