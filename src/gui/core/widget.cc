#include <widget.h>

namespace nglui{

Widget::Widget(int w,int h)
  : Widget(std::string(),w,h){
}

Widget::Widget(const std::string& text)
  : Widget(text,0,0) {
}

Widget::Widget(const std::string& text, int width, int height)
  : INHERITED(width,height),text_(text)
{
   GraphContext*canvas=GraphDevice::getInstance()->getPrimaryContext();
   TextExtents te;
   canvas->set_font_size(getFontSize());
   canvas->get_text_extents(text,te);
   if(width<=0)
      width=te.width+getFontSize();
   if(height<=0)
      height=te.height+getFontSize();
   setBound(0,0,width,height);
   text_alignment_=DT_LEFT|DT_VCENTER;
}

Widget::Widget(const std::string& text, RECT& bound)
  : Widget(text) {
  setBound(bound);
}

Widget::~Widget() {
}

void Widget::setText(const std::string& text) {
   text_ = text;
   invalidate(nullptr);
}

const std::string& Widget::getText() const {
  return text_;
}

void Widget::setAlignment(int align){
    text_alignment_=align;
    invalidate(nullptr);
}

int Widget::getAlignment(){
    return text_alignment_;
}

}
