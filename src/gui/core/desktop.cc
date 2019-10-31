#include<desktop.h>
#include<toolbar.h>
#include<windowmanager.h>
#include<ngl_ir.h>
#include<ngl_log.h>

namespace nglui{

Desktop::Desktop():Window(0,0,
   GraphDevice::getInstance()->getScreenWidth(),
   GraphDevice::getInstance()->getScreenHeight()){
   onkeypress=nullptr;
   onmessage=nullptr;
   clearFlag(Attr::ATTR_BORDER);
   setBgColor(0);
   statusbar_=new ToolBar(GraphDevice::getInstance()->getScreenWidth(),40);
   RefPtr<LinearGradient>linpat(LinearGradient::create(0, 0, 1280, 0));
   linpat->add_color_stop_rgba ( .0, 1, 1, 1, 0);
   linpat->add_color_stop_rgba ( .2, 0, 1, 0, 0.5);
   linpat->add_color_stop_rgba ( .4, 1, 1, 1, 0);
   linpat->add_color_stop_rgba ( .6, 0, 0, 1, 0.5);
   linpat->add_color_stop_rgba ( .8, 1, 1, 1, 0);

   statusbar_->setBgColor(0xFFAAAAAA);
   statusbar_->setBgPattern(linpat);
   statusbar_->getLayout()->setMarginRight(150);
   statusbar_->addButton("Network");
   statusbar_->addButton("Settings");
   statusbar_->addButton("Menu");
   addChildView(statusbar_);
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
    if(k.getKeyCode()==NGL_KEY_POWER)
        exit(0);
    return false;
}

bool Desktop::onMessage(DWORD msgid,DWORD wParam,ULONG lParam){
    bool rc=Window::onMessage(msgid,wParam,lParam);
    if(false==rc&&onmessage)
        onmessage(msgid,wParam,lParam);
}

}//namespace
