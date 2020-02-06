#ifndef __DESKTOP_H__
#define __DESKTOP_H__
#include<window.h>
namespace nglui{

typedef bool (*KeyHandler)(int);
typedef void (*MessageHandler)(DWORD,DWORD,ULONG);
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
