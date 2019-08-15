#include<desktop.h>
#include<windowmanager.h>
#include<ngl_log.h>

namespace nglui{

Desktop::Desktop():Window(0,0,
   GraphDevice::getInstance()->getScreenWidth(),
   GraphDevice::getInstance()->getScreenHeight()){
   onkeypress=nullptr;
   onmessage=nullptr;
}

void Desktop::draw(bool flip){
   if(nullptr==canvas){
        canvas=GraphDevice::getInstance()->getPrimaryContext();
    }
    onDraw(*canvas);
    if(flip)canvas->flip();
}

void Desktop::onDraw(GraphContext&canvas){
   //do nothing 
}

void Desktop::setKeyListener(KeyHandler fun){
    onkeypress=fun;
}

void Desktop::setMessageListener(MessageHandler onmsg){
    onmessage=onmsg;
}

bool Desktop::onKeyRelease(KeyEvent&k){
    if(onkeypress!=nullptr)
      return  onkeypress(k.getKeyCode());
    return false;
}

bool Desktop::onMessage(DWORD msgid,DWORD wParam,ULONG lParam){
    bool rc=Window::onMessage(msgid,wParam,lParam);
    if(false==rc&&onmessage)
      onmessage(msgid,wParam,lParam);
}

}//namespace
