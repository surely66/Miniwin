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

#include <windowmanager.h>
#include <ngl_msgq.h>
#include <ngl_os.h>
#include <ngl_log.h>
#include <ngl_ir.h>
#include <ngl_timer.h>

NGL_MODULE(WINDOWMANAGER)

namespace nglui {
// Initialize the instance of the singleton to nullptr
WindowManager* WindowManager::instance_ = nullptr;

WindowManager::WindowManager()
{
     GraphDevice::getInstance();
}

WindowManager*WindowManager::getInstance(){
    if(nullptr==instance_){
        DWORD threadid;
        instance_=new WindowManager();
    }
    return instance_;
};

WindowManager::~WindowManager() {
    windows_.erase(windows_.begin(),windows_.end());
}

void WindowManager::sendMessage(Window*w,DWORD msgid,DWORD wp,ULONG lp,DWORD delayedtime){
    for(auto win=windows_.begin();win!=windows_.end();win++){
       if((*win).get()==w){
           sendMessage(*win,msgid,wp,lp,delayedtime);
           return ;
       }
    }
}

void WindowManager::sendMessage(std::shared_ptr<View>v,DWORD msgid,DWORD wp,ULONG lp,DWORD delayedtime){
    UIMSG msg={v,msgid,wp,lp,delayedtime};
    msg.view=v;
    msg.msgid=msgid;
    msg.wParam=wp;    msg.lParam=lp;
    if(msgid==View::WM_INVALIDATE){
       invalidate_views.emplace(v);
       return ;
    }
    if(delayedtime!=0){
       NGL_RunTime tnow;
       nglGetRunTime(&tnow);
       msg.time=tnow.uiMilliSec+tnow.uiMicroSec/1000+delayedtime;
       delayed_msgq_.push(msg); 
       return;
    }
    msg_queue_.push(msg);
}

bool WindowManager::hasDelayedMessage(){
    NGL_RunTime tnow;
    UIMSG msg;
    if(delayed_msgq_.empty())return false;
    nglGetRunTime(&tnow);
    DWORD nowms=tnow.uiMilliSec+tnow.uiMicroSec/1000;
    msg=delayed_msgq_.top();
    return msg.time<nowms;
}
void WindowManager::popMessage(){
    UIMSG msg;
    std::shared_ptr<View>sp;
    {
       if(msg_queue_.size()){
          msg=msg_queue_.front();
          std::weak_ptr<View>wp=msg.view;
          if(wp.use_count()){
             sp=wp.lock();
             if(View::WM_DESTROY==msg.msgid){
                 removeWindow(std::static_pointer_cast<Window>(sp));
             }
          }
          msg_queue_.pop();
      }else if(hasDelayedMessage()){
          msg=delayed_msgq_.top();
          delayed_msgq_.pop();
          std::weak_ptr<View>wp=msg.view;
          if(wp.use_count()){
             sp=wp.lock();
          }
      }
    }
    if(sp!=nullptr)sp->onMessage(msg.msgid,msg.wParam,msg.lParam);
}

void WindowManager::addWindow(Window*w){
    std::shared_ptr<Window>wp(w);
    windows_.push_back(wp);
    SIZE sz;
    sz.x=w->getWidth();
    sz.y=w->getHeight();
    w->onResize(sz);
    NGLOG_VERBOSE("w=%p windows.size=%d",w,windows_.size());
}

void WindowManager::removeWindow(std::shared_ptr<Window>w){
   for(auto win=windows_.begin();win!=windows_.end();win++){
       if((*win)==w){
            windows_.erase(win);
            break;
       }
   } 
   NGLOG_VERBOSE("w=%p windows.size=%d",w.get(),windows_.size());
}

void WindowManager::onResize(int width, int height) {
  SIZE new_size ={width,height};
  // Notify all children
  for (auto wind : windows_) {
    if (wind->isFocused() == true) {
      wind->onResize(new_size);
    }
  }
}

void WindowManager::onReposition(uint32_t x, uint32_t y) {
}

void WindowManager::onBtnPress(uint32_t x, uint32_t y, uint32_t modi) {
  Event evt ;//= Event::Make(Event::Type::M_PRESS, x, y, modi);
  // Notify the focused child
  for (auto& wind : windows_) {
    if (wind->isFocused() == true) {
      wind->onMousePress(evt);
    }
  }
}

void WindowManager::onBtnRelease(uint32_t x, uint32_t y, uint32_t modi) {
  Event evt;// = Event::Make(Event::Type::M_RELEASE, x, y, modi);
  // Notify the focused child
  for (auto& wind : windows_) {
    if (wind->isFocused() == true) {
      wind->onMouseRelease(evt);
    }
  }
}

void WindowManager::onMotion(uint32_t x, uint32_t y, uint32_t modi) {
  Event evt;// = Event::Make(Event::Type::M_MOTION, x, y, modi);
  // Notify the focused child
  for (auto& wind : windows_) {
    if (wind->isFocused() == true) {
      wind->onMouseMotion(evt);
    }
  }
}

void WindowManager::onKeyPress(uint32_t key) {
  KeyEvent evt(key,NGL_KEY_PRESSED,0);// = Event::Make(Event::Type::K_DOWN, key);
  // Notify the focused child
  for (auto wind=windows_.rbegin();wind!=windows_.rend();wind++) {
    if ( (*wind)->hasFlag(View::Attr::ATTR_FOCUSABLE) ) {
        (*wind)->onKeyPress(evt);
        return;
    }
  }
}

void WindowManager::onKeyRelease(uint32_t key) {
  KeyEvent evt(key,NGL_KEY_RELEASE,0);// = Event::Make(Event::Type::K_UP, key);
  // Notify the focused child
  for (auto wind=windows_.rbegin() ;wind!= windows_.rend();wind++) {
    if ( (*wind)->hasFlag(View::Attr::ATTR_FOCUSABLE) ) {
        (*wind)->onKeyRelease(evt);
        return;
    }
  }
}

void WindowManager::onKeyChar(uint32_t key) {
    KeyEvent evt(key,0,0);// = Event::Make(Event::Type::K_CHAR, key);
    // Notify the focused child
    for (auto wind=windows_.rbegin() ;wind!=windows_.rend();wind++) {
        if ((*wind)->isFocused() == true) {
            (*wind)->onKeyChar(evt);
        }
    }
}

bool WindowManager::hasDirtyWindows(){
    for (auto wind : windows_)
        if(wind->isDirty())return true;
    return false;//invalidate_views.size()>0;
}

void WindowManager::drawWindows() {
    // Notify the focused child to draw on this canvas
    int dirty=0;
    for (auto wind : windows_) {
        if(wind->isDirty()){
           wind->draw(false);
	   dirty++;
	   NGLOG_DEBUG("window %p is dirty",wind);
	}
    }
    /*for(auto v:invalidate_views){
        GraphContext*c=v->getCanvas();
        NGLOG_DEBUG_IF(c==nullptr,"canvas=NULL"); 
        if(c==nullptr)continue;
        RefPtr<GraphContext>canvas(c);
        //v->clip(*canvas);
        v->onDraw(*canvas);
    }*/
    NGLOG_DEBUG_IF(dirty||invalidate_views.size()>1,"invalidate_views.size=%d dirty=%d",invalidate_views.size(),dirty);
    invalidate_views.clear();
    if(dirty)GraphDevice::getInstance()->flip(nullptr);
}

int WindowManager::hasEvents(){
    int rc=0;
    if(msg_queue_.size())rc|=1;
    if(hasDelayedMessage())rc|=2;
    if(hasDirtyWindows())rc|=4;
    if(GraphDevice::getInstance()->needCompose())rc|=8;
    return rc; 
}

void WindowManager::runOnce(){
    int evts=hasEvents();
    NGLOG_VERBOSE("events=%x",evts);
    if (evts&4)drawWindows();
    if(evts&0x0C)
       GraphDevice::getInstance()->ComposeSurfaces();
    if(evts&3)popMessage();
}

void WindowManager::run(){
    while(true){
        runOnce();
    }
}

}  // namespace ui
