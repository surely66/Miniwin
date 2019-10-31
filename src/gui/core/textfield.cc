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
#include "textfield.h"
#include <app.h>
#include <ngl_log.h>

NGL_MODULE(TEXTFIELD)

namespace nglui {

TextField::TextField(const std::string& text)
  : TextField(text,0,0) {
//  internal_name_ = VIEW_NAME;
}

TextField::TextField(const std::string& text, int width, int height)
  : INHERITED(text, width, height) {
    multiline=false;
}

TextField::~TextField() {
}

void TextField::setMultiLine(bool m){
     if(m)
         text_alignment_|=DT_MULTILINE;
     else
         text_alignment_&=(~DT_MULTILINE);
}

void TextField::setImage(int dir,const std::string&resname){
    if(resname.empty()==false)
        images[dir]=App::getInstance().loadImage(resname);
}

void TextField::onDraw(GraphContext& canvas) {
    RECT rcimg,rect=getClientRect();
    canvas.set_color(getBgColor());
    canvas.draw_rect(rect);
    // Border
    if(hasFlag(Attr::ATTR_BORDER)){
        canvas.set_color(getFgColor());
        canvas.set_line_width(1);
        canvas.rectangle(0,0,getWidth(),getHeight());
        canvas.stroke();
    }
    if(images[2]){
        rcimg=rect;
        rcimg.width=images[2]->get_width();
        canvas.draw_image(images[2],&rcimg,nullptr,ST_CENTER_INSIDE);
        rect.x+=rcimg.width; 
        rect.width-=rcimg.width;
    }
    // Text
    canvas.set_color(fg_color_);
    canvas.set_font_size(getFontSize());
    canvas.draw_text(rect,text_.c_str(),getAlignment());
}

const SIZE& TextField::getPreferSize() {
  prefer_size_.set(getWidth(), getHeight());
  return prefer_size_;
}

}  // namespace ui
