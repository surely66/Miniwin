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

#include <groupview.h>
#include <ngl_log.h>
#include <ngl_ir.h>
#include <uievents.h>
#include <pixman.h>

NGL_MODULE(GroupView)

namespace nglui {

GroupView::GroupView(int w,int h)
  : GroupView(0,0,w,h) {
   //internal_name_ = VIEW_NAME;
}

GroupView::GroupView(int x,int y,int w,int h)
:INHERITED(w,h){
   setBound(x,y,w,h);
   setFlag(Attr::ATTR_TRANSPARENT);
   setFlag(Attr::ATTR_ENABLE);
   //setFlag(Attr::ATTR_BORDER);
}
GroupView::~GroupView() {
}

const SIZE& GroupView::getPreferSize(){
    return prefer_size_;
}

bool GroupView::isDirty(){
    NGLOG_VERBOSE("GroupView %x region_empty=%d",this,pixman_region32_not_empty(invalid_region_));
    return hasFlag(Attr::ATTR_VISIBLE)&&pixman_region32_not_empty(invalid_region_);
}

void GroupView::onDraw(GraphContext& canvas) {
    // Draw the background color, if enabled
    canvas.reset_clip();
    clip(canvas);
    INHERITED::onDraw(canvas);
    for(auto child : children_){
        RECT rect=child->getBound();
        RefPtr<GraphContext>subcanvas(child->getCanvas());
	//(new GraphContext(canvas,child->getX(),child->getY(),child->getWidth(),child->getHeight()));
        subcanvas->set_font_face(GraphDevice::getInstance()->getFont());
        subcanvas->set_antialias(ANTIALIAS_GRAY);
        NGLOG_VERBOSE("%p draw view %p   %d,%d-%d,%d",this,child.get(),rect.x,rect.y,rect.width,rect.height);
        if (child->hasFlag(Attr::ATTR_TRANSPARENT) == true || child->isVisible() == true) {
            child->clip(*subcanvas);
            if (child->getAnimateState() == AnimState::ANIM_NONE) {
                child->onDraw(*subcanvas);
            } else {
                child->onDraw(*subcanvas);
            }
        }child->resetClip();
    }
    resetClip();
}

void GroupView::onAnimate() {
    for (auto child : children_) {
        child->onAnimate();
    }
}

void GroupView::onResize(SIZE& size) {
    for (auto child : children_) {
        // Notify all children to do resizing
        if (layout_ != nullptr) {
           // Invoke the layout
           layout_->onLayout(this);
        }
        // Call child's OnResize();
        child->onResize(size);
    }
}

bool GroupView::onMousePress(Event& evt) {
    // Iterate each child to find out which one to distribute this event
    for (auto child : children_) {
       // Check if it's enabled
       if (child->isEnable() == false) {
           continue;
       }
       RECT rb=child->getBound();
       // Check if it's intersected, if not, just ignore this view
       if (rb.intersect(evt.getX(), evt.getY()) == false) {
           continue;
       }
       if (child->hasFlag(Attr::ATTR_TRANSPARENT) == true) {
           // If it's a LAYOUT view, distribute this event to it
           // And let it do the rest
           child->onMousePress(evt);
           continue;
       }
       if (child->onMousePress(evt) == true) {
           // If it's not a LAYOUT view, but a WIDGET one.
           // Try to call its handler to see whether it will be handled.
           // If so, terminate this process.
           // If not, continue to check out next WIDGET view.
           return true;
       }
    }
    return false;
}

bool GroupView::onMouseRelease(Event& evt) {
    // Iterate each child to find out which one to distribute this event
    for (auto child : children_) {
        // Check if it's enabled
        if (child->isEnable() == false) {
            continue;
        }
        RECT rb=child->getBound();
        // Check if it's intersected, if not, just ignore this view
        if (rb.intersect(evt.getX(), evt.getY()) == false) {
            continue;
        }
        if (child->hasFlag(Attr::ATTR_TRANSPARENT) == true) {
            // If it's a LAYOUT view, distribute this event to it
            // And let it do the rest
            child->onMouseRelease(evt);
            continue;
        }
        if (child->onMouseRelease(evt) == true) {
            // If it's not a LAYOUT view, but a WIDGET one.
            // Try to call its handler to see whether it will be handled.
            // If so, terminate this process.
            // If not, continue to check out next WIDGET view.
            return true;
        }
    }
    return false;
}

bool GroupView::onMouseMotion(Event& evt) {
    // Iterate each child to find out which one to distribute this event
    for (auto child : children_) {
       // Check if it's enabled
       if (child->isEnable() == false) {
           continue;
       }
       RECT rb=child->getBound();
       // Check if it's intersected, if not, just ignore this view
       if (rb.intersect(evt.getX(), evt.getY()) == false) {
            continue;
       }
       if (child->hasFlag(Attr::ATTR_TRANSPARENT) == true) {
       // If it's a LAYOUT view, distribute this event to it
       // And let it do the rest
       child->onMouseMotion(evt);
       continue;
    }
    if (child->onMouseMotion(evt) == true) {
        // If it's not a LAYOUT view, but a WIDGET one.
        // Try to call its handler to see whether it will be handled.
        // If so, terminate this process.
        // If not, continue to check out next WIDGET view.
        return true;
    }
  }
  return false;
}

bool GroupView::onKeyPress(KeyEvent& evt) {
    // Iterate each child to find out which one to distribute this event
    for (auto child : children_) {
        // Check if it's enabled
        if (child->isEnable() == false) {
            continue;
        }
        if (child->hasFlag(Attr::ATTR_TRANSPARENT) == true) {
            // If it's a LAYOUT view, distribute this event to it
            // And let it do the rest
            child->onKeyPress(evt);
            continue;
        }
        if (child->onKeyPress(evt) == true) {
            // If it's not a LAYOUT view, but a WIDGET one.
            // Try to call its handler to see whether it will be handled.
            // If so, terminate this process.
            // If not, continue to check out next WIDGET view.
            return true;
        }
    }
    return false;
}

View*GroupView::getFocused(){
    View*first=nullptr;//first focusable view
    for(auto it=children_.begin();it!=children_.end();it++){
       if(nullptr==first&&(*it)->hasFlag(Attr::ATTR_FOCUSABLE))
           first=it->get();
       if((*it)->hasFlag(Attr::ATTR_FOCUSED)){
           return it->get();
       }
    }
    return first;
}

View* GroupView::getNextFocus(View*cv,int key){
    int idx=getViewOrder(cv);
    int dir=0,odx=idx;
    RECT rc;
    switch(key){
    case NGL_KEY_LEFT:
    case NGL_KEY_UP:dir=-1;break;
    case NGL_KEY_RIGHT:
    case NGL_KEY_DOWN:dir=1;break;
    default:dir=1;break;
    }
    for(int i=0;i<getChildrenCount();i++){
        idx+=dir;
        if(idx<0||idx>=getChildrenCount())return nullptr;
        View*v=getChildView(idx);
        if(v->hasFlag(Attr::ATTR_FOCUSABLE)&&v->isEnable()){
             if(cv){
                 cv->clearFlag(Attr::ATTR_FOCUSED);
                 cv->invalidate(nullptr);
             }
             v->setFlag(Attr::ATTR_FOCUSED);
             NGLOG_VERBOSE("changefocuse from %p to %p %d->%d",cv,v,odx,idx);
             return v;
        }
    }
    return nullptr;
}

bool GroupView::onKeyRelease(KeyEvent& evt) {
    // Iterate each child to find out which one to distribute this event
    View*nfv;
    View*cfv=getFocused();
    if(nullptr==cfv)cfv=getNextFocus(nullptr,NGL_KEY_RIGHT);
    if( cfv && cfv->onKeyRelease(evt)==false){
        View* nfv=getNextFocus(cfv,((KeyEvent&)evt).getKeyCode());
        if(nfv)nfv->invalidate(nullptr);
    }
    return false;
}

bool GroupView::onKeyChar(KeyEvent& evt) {
    // Iterate each child to find out which one to distribute this event
    for (auto child : children_) {
        // Check if it's enabled
        if (child->isEnable() == false) {
            continue;
        }
        if (child->hasFlag(Attr::ATTR_TRANSPARENT) == true) {
            // If it's a LAYOUT view, distribute this event to it
            // And let it do the rest
            child->onKeyChar(evt);
            continue;
        }
        if (child->onKeyChar(evt) == true) {
            // If it's not a LAYOUT view, but a WIDGET one.
            // Try to call its handler to see whether it will be handled.
            // If so, terminate this process.
            // If not, continue to check out next WIDGET view.
            return true;
        }
   }
  return false;
}

}  // namespace ui
