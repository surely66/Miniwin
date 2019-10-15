#include <toastwindow.h>
#include <windows.h>
#include <ngl_log.h>
NGL_MODULE(TOAST)

namespace nglui{

ToastWindow::ToastWindow(int w,int h,int timeout):Window(0,0,w,h){
    timeout_=timeout;
    time_elapsed=0;
    clearFlag(Attr::ATTR_FOCUSABLE);
    sendMessage(WM_TIMER,TIMER_ID,0,500);
}

bool ToastWindow::onMessage(DWORD msg,DWORD wp,ULONG lp){
    if(msg==WM_TIMER && wp==TIMER_ID){
         time_elapsed+=500;
         if(time_elapsed>=timeout_){
             sendMessage(WM_DESTROY,0,0);
             time_elapsed=0;
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
}

ToastWindow*ToastWindow::makeText(const std::string&txt,UINT timeout){
    int sw,sh,tw,th;
    GraphDevice::getInstance()->getScreenSize(sw,sh);
    GraphDevice::getInstance()->getPrimaryContext()->set_font_size(20);
    GraphDevice::getInstance()->getPrimaryContext()->get_text_size(txt,&tw,&th);
    tw+=th*4;th+=th;
    return makeWindow(tw,th,0,[&](Window&w,int){
           w.addChildView(new TextField(txt,tw,th));
           w.setPos((sw-tw)/2,10);
       },timeout);
}
}//namespace

