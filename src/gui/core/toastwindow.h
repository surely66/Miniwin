#ifndef __TOAST_WINDOW_H__
#define __TOAST_WINDOW_H__
#include <window.h>

namespace nglui{

class ToastWindow:public Window{
enum{
    LENGTH_SHORT=1000,
    LENGTH_LONG=3000
};
public:
    ToastWindow(const std::string&txt,int w,int h,int timeout);
    virtual bool onMessage(DWORD msg,DWORD wp,ULONG lp)override;
    static ToastWindow*makeText(const std::string&txt,int long timeout=LENGTH_SHORT);
private:
    const static int TIMER_ID=0x1000;
    DWORD timeout_;
    DISALLOW_COPY_AND_ASSIGN(ToastWindow);
};
typedef ToastWindow  Toast;

}//namespace

#endif
