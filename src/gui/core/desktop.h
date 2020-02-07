#ifndef __DESKTOP_H__
#define __DESKTOP_H__
#include<window.h>
namespace nglui{

DECLARE_UIEVENT(bool,MessageHandler,DWORD,DWORD,ULONG);
DECLARE_UIEVENT(bool,KeyHandler,int);
class Desktop:public Window{
private:
   KeyHandler onkeypress;
   MessageHandler onmessage;
   class ToolBar*statusbar_;
public:
   Desktop();
   virtual void setKeyListener(KeyHandler fun);
   virtual void setMessageListener(MessageHandler onmsg);
protected:
   virtual bool onKeyRelease(KeyEvent&k)override;
   virtual bool onMessage(DWORD msgid,DWORD wParam,ULONG lParam)override;
DISALLOW_COPY_AND_ASSIGN(Desktop);
};

}//namespace
#endif
