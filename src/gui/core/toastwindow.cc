#include <toastwindow.h>
#include <windows.h>

namespace nglui{

ToastWindow::ToastWindow(const std::string&txt,int w,int h,int timeout):Window(0,0,w,h){
    addChildView(new TextField(txt,w,h));
    timeout_=timeout;
    clearFlag(Attr::ATTR_FOCUSABLE);
    sendMessage(WM_TIMER,TIMER_ID,0,timeout_);
}

bool ToastWindow::onMessage(DWORD msg,DWORD wp,ULONG lp){
    if(msg==WM_TIMER && wp==TIMER_ID){
         sendMessage(WM_DESTROY,0,0);
         return true;
    }
    return Window::onMessage(msg,wp,lp);
}

ToastWindow*ToastWindow::makeText(const std::string&txt,int long timeout){
    int sw,sh,tw,th;
    GraphDevice::getInstance()->getScreenSize(sw,sh);
    GraphDevice::getInstance()->getPrimaryContext()->set_font_size(20);
    GraphDevice::getInstance()->getPrimaryContext()->get_text_size(txt,&tw,&th);
    tw+=th*4;th+=th;
    ToastWindow*w=new ToastWindow(txt,tw,th,timeout);
    w->setPos((sw-tw)/2,10);
    return w;
}
}

