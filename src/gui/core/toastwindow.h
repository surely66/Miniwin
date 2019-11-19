#ifndef __TOAST_WINDOW_H__
#define __TOAST_WINDOW_H__
#include <window.h>

namespace nglui{

class ToastWindow:public Window{
enum{
    LENGTH_SHORT=2000,
    LENGTH_LONG=4000
};
typedef std::function<void(Window&,int)>OnCreateContentListener;
public:
    ToastWindow(int w,int h,int timeout);
    ~ToastWindow();
    virtual View& setPos(int x,int y);
    virtual bool onMessage(DWORD msg,DWORD wp,ULONG lp)override;
    static ToastWindow*makeWindow(int w,int h,int flags,OnCreateContentListener oncreate,UINT timeout=LENGTH_SHORT);
    static ToastWindow*makeText(const std::string&txt,UINT timeout=LENGTH_SHORT);
private:
    const static int TIMER_ID=0x1000;
    static std::map<Window*,POINT>winpos_;
    static std::vector<Window*>toasts_;
    DWORD timeout_;
    DWORD time_elapsed;
    virtual bool onKeyRelease(KeyEvent&k)override;
    DISALLOW_COPY_AND_ASSIGN(ToastWindow);
};
typedef ToastWindow  Toast;

}//namespace

#endif
