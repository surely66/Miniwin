#include<imagebutton.h>
#include<ngl_log.h>
#include<app.h>
NGL_MODULE(IMAGEBUTTON)
namespace nglui{

ImageButton::ImageButton(int w,int h):ImageButton(std::string(),w,h){

}

ImageButton::ImageButton(const std::string&txt,int w,int h):INHERITED(txt,w,h){
   setFlag(Attr::ATTR_FOCUSABLE);
   setFlag(Attr::ATTR_ENABLE);
}

void ImageButton::setHotImage(const std::string&respath){
    if(respath.empty()==false)
        imghot=App::getInstance().loadImage(respath);
    invalidate(nullptr);
}

void ImageButton::onDraw(GraphContext&canvas){
    RECT rect=getClientRect();
    canvas.set_font_size(getFontSize());
    canvas.set_color(getBgColor());
    canvas.draw_rect(rect);
    if(hasFlag(Attr::ATTR_FOCUSED)==false){
        if(img_)canvas.draw_image(img_,&rect,nullptr,ST_CENTER_INSIDE);
        canvas.set_color(getFgColor());
        if(text_.empty()==false){
            canvas.draw_text(rect,text_,DT_CENTER|DT_BOTTOM);
        }
    }else{
        if(imghot)canvas.draw_image(imghot,&rect,nullptr,ST_CENTER_INSIDE);
    }
}

}
