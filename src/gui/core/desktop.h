#ifndef __DESKTOP_H__
#define __DESKTOP_H__
#include<window.h>
namespace nglui{

typedef std::function<bool (int)>KeyHandler;
typedef std::function<void(DWORD,DWORD,ULONG)>MessageHandler;
class Desktop:public Window{
private:
   KeyHandler onkeypress;
   MessageHandler onmessage;
public:
   Desktop();
   virtual void setKeyListener(KeyHandler fun);
   virtual void setMessageListener(MessageHandler onmsg);
protected:
   virtual void draw(bool)override;
   virtual void onDraw(GraphContext&canvas)override;
   virtual bool onKeyRelease(KeyEvent&k)override;
   virtual bool onMessage(DWORD msgid,DWORD wParam,ULONG lParam)override;
DISALLOW_COPY_AND_ASSIGN(Desktop);
};

}//namespace
#endif