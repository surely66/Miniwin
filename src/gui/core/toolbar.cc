#include<toolbar.h>
#include<linearlayout.h>
#include<textfield.h>
#include<app.h>
#include<ngl_log.h>
#include<ngl_ir.h>
NGL_MODULE(TOOLBAR)
namespace nglui{

ToolBar::ToolBar(int width, int height):INHERITED(width,height){
   setFontSize(20); 
   setFgColor(0xFFFFFFFF);
   index_=-1;
   setLayout(new LinearLayout())->setGravity(GRAVITY::RIGHT);
}

//#define TEXTFIELD_AS_BUTTON 1
int ToolBar::getButtonCount(){
    return buttons.size();
}

void ToolBar::setIndex(int idx){
#ifndef TEXTFIELD_AS_BUTTON
    if(idx!=index_){
        RECT r=getClientRect();
        if(index_>=0&&index_<=buttons.size()){
            r.x=buttons[index_].pos;
            r.width=buttons[index_].width;
            invalidate(&r);
        }
        index_=idx;
        if(index_>=0&&index_<=buttons.size()){
            r.x=buttons[index_].pos;
            r.width=buttons[index_].width;
            invalidate(&r);
        }
    }
#endif
}

int ToolBar::getIndex(){
    return index_;
}

void ToolBar::clearButtons(){
#ifndef TEXTFIELD_AS_BUTTON
    buttons.clear();
#else
    removeChildren();
#endif
    index_=-1;
    invalidate(nullptr);
}

bool ToolBar::onKeyRelease(KeyEvent&k){
    int idx;
#ifndef TEXTFIELD_AS_BUTTON
    switch(k.getKeyCode()){
    case NGL_KEY_LEFT:
        idx=(index_-1+buttons.size())%buttons.size();
        setIndex(idx);
        break;
    case NGL_KEY_RIGHT:
        idx=(index_+1+buttons.size())%buttons.size();
        setIndex(idx);
        break;
    default:return false;
    }
#endif
    return false;
}
void ToolBar::addButton(const std::string&txt,int x,int w){
    addButton(std::string(),txt,x,w);
}
void ToolBar::addButton(const std::string&img,const std::string&txt,int x,int w){
#ifndef TEXTFIELD_AS_BUTTON
    BUTTON btn;
    int lastx=0;
    if(img.length()>2){
        btn.image=App::getInstance().loadImage(img);
    }
    btn.text=txt;

    GraphContext*canvas=GraphDevice::getInstance()->getPrimaryContext();
    TextExtents te;
    canvas->set_font_size(getFontSize());
    canvas->get_text_extents(txt,te);

    if(buttons.size()){
        lastx=buttons.back().pos+buttons.back().width;
    }
    btn.pos=x>0?x:lastx;
    btn.width=(w<0)?(te.width+2*te.height):w;
    if( btn.image && (w<0) ){
        btn.width+=btn.image->get_width();
    }
    NGLOG_VERBOSE("%p button %s %d:%d ",this,txt.c_str(),btn.pos,btn.width);
    buttons.push_back(btn);
    invalidate(nullptr);
#else
    TextField*tf=new TextField(txt,w,getHeight());
    NGLOG_VERBOSE("%p button %s %dx%d w=%d",this,txt.c_str(),getWidth(),getHeight(),w);
    tf->setFgColor(0xFFFF0000);
    tf->setBgColor(getBgColor());
    addChildView(tf);
#endif
}

void ToolBar::onDraw(GraphContext&canvas){
    RECT rect=getClientRect();
    INHERITED::onDraw(canvas);
#ifndef TEXTFIELD_AS_BUTTON 
    canvas.set_font_size(getFontSize());
    for(auto b:buttons){
        rect.x=b.pos;
        if(b.image){
            canvas.draw_image(b.image,b.pos,(getHeight()-b.image->get_height())/2);
            rect.x+=b.image->get_width()+4;
        }
        canvas.set_color(getFgColor());
        canvas.draw_text(rect,b.text,DT_LEFT|DT_VCENTER);
    }
#endif
}

}//end namespace

