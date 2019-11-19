#include <toastwindow.h>
#include <windows.h>
#include <ngl_log.h>
NGL_MODULE(TOAST)

namespace nglui{

std::map<Window*,POINT>ToastWindow::winpos_;
std::vector<Window*>ToastWindow::toasts_;
 
ToastWindow::ToastWindow(int w,int h,int timeout):Window(0,0,w,h){
    timeout_=timeout;
    time_elapsed=0;
    clearFlag(Attr::ATTR_FOCUSABLE);
    sendMessage(WM_TIMER,TIMER_ID,0,500);
    toasts_.push_back(this);
}

ToastWindow::~ToastWindow(){
    toasts_.erase(std::find(toasts_.begin(),toasts_.end(),this));
}

View& ToastWindow::setPos(int x,int y){
    int wy=50;
    if(winpos_.find(this)==winpos_.end()){
        for(auto w:toasts_){
           if(w==this)break;
           wy+=w->getHeight()+1;
        }
    }else wy=y;
    winpos_[this].x=x;
    winpos_[this].y=wy;
    Window::setPos(x,wy);
    NGLOG_DEBUG("Toast %p pos=(%d,%d)",this,x,wy);
}

bool ToastWindow::onMessage(DWORD msg,DWORD wp,ULONG lp){
    if(msg==WM_TIMER && wp==TIMER_ID){
         time_elapsed+=500;
         if(time_elapsed>=timeout_){
             sendMessage(WM_DESTROY,0,0);
             time_elapsed=0;
             winpos_.erase(this);
         }
         sendMessage(msg,wp,lp,500);
         return true;
    }
    return Window::onMessage(msg,wp,lp);
}

bool ToastWindow::onKeyRelease(KeyEvent&k){
    time_elapsed=0;
    return Window::onKeyRelease(k);
}

ToastWindow*ToastWindow::makeWindow(int width,int height,int flags,OnCreateContentListener oncreate,UINT timeout){
    ToastWindow*w=nullptr;
    if(oncreate){
         w=new ToastWindow(width,height,timeout);
         oncreate(*w,flags);
    }
    return w;
}

ToastWindow*ToastWindow::makeText(const std::string&txt,UINT timeout){
    int sw,sh,tw,th;
    GraphDevice::getInstance()->getScreenSize(sw,sh);
    GraphDevice::getInstance()->getPrimaryContext()->set_font_size(20);
    GraphDevice::getInstance()->getPrimaryContext()->get_text_size(txt,&tw,&th);
    tw+=th*4;th+=th;
    return makeWindow(tw,th,0,[&](Window&w,int){
           TextField*tf=new TextField(txt,tw,th);
           tf->setMultiLine(true);
           w.addChildView(tf);
           w.setPos((sw-tw)/2,50);
       },timeout);
}
}//namespace

