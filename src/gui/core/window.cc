/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <window.h>
#include <windowmanager.h>
#include <ngl_log.h>

//#pragma GCC diagnostic ignored "-fpermissive"

NGL_MODULE(WINDOW)

namespace nglui {

Window::Window(int x,int y,int width,int height)
  : INHERITED(x,y,width,height){
    // Set the boundary
    setBound(x, y, width, height);
    setFlag(Attr::ATTR_BORDER);
    // Set focusable flag
    setFlag(Attr::ATTR_FOCUSABLE);
    clearFlag(Attr::ATTR_TRANSPARENT);
    // Do the resizing at first time in order to invoke the OnLayout
    SIZE size={width,height};
    onResize(size);
    NGLOG_VERBOSE("%p",this);
    canvas=nullptr;
    WindowManager::getInstance()->addWindow(this);
}

void Window::show(){
    NGLOG_DEBUG("visible=%d",hasFlag(Attr::ATTR_VISIBLE));
    if(!hasFlag(Attr::ATTR_VISIBLE)){
        setFlag(Attr::ATTR_VISIBLE);
        //draw(true);
        invalidate(nullptr);
    }
}

void Window::hide(){
    clearFlag(Attr::ATTR_VISIBLE);
}

View& Window::setPos(int x,int y){
    if(x!=bound_.x || y!=bound_.y){
        bound_.x=x;
        bound_.y=y;
        if(canvas){
           canvas->set_position(x,y);
           canvas->flip();
        }
    }
    return *this;
}

View& Window::setSize(int cx,int cy){
    if(cx!=getWidth()||cy!=getHeight()){
        bound_.width=cx;
        bound_.height=cy;
        SIZE sz={cx,cy};
        onResize(sz);
    }
    return *this;
}

GraphContext*Window::getCanvas(){
    if(nullptr==canvas){
        canvas=GraphDevice::getInstance()->createContext(bound_);
    }
    return canvas;
}
void Window::draw(bool flip){
    onDraw(*getCanvas());
    if(flip)canvas->flip();
}

Window::~Window() {
    delete canvas;
    NGLOG_VERBOSE("%p",this);
}

bool Window::onKeyRelease(KeyEvent& evt){
    if(INHERITED::onKeyRelease(evt))
        return true;
    switch(evt.getKeyCode()){
    case NGL_KEY_ESCAPE:
         NGLOG_DEBUG("recv NGL_KEY_ESCAPE");
         sendMessage(WM_DESTROY,0,0);
         break;
    } 
    return false;
}

void Window::sendMessage(DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime){
    WindowManager::getInstance()->sendMessage(this,msgid,wParam,lParam,delayedtime);
}

void Window::sendMessage(std::shared_ptr<View>w,DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime){
    WindowManager::getInstance()->sendMessage(w,msgid,wParam,lParam,delayedtime);
}

void  closeWindow(Window*w){
    w->sendMessage(View::WM_DESTROY,0,0);
}

}  // namespace ui
