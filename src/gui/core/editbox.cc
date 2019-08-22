#include<editbox.h>
#include<ngl_ir.h>
#include<ngl_log.h>

NGL_MODULE(EDITBOX)

namespace nglui{

EditBox::EditBox(int w,int h):EditBox(std::string(),w,h){
}

EditBox::EditBox(const std::string&txt,int w,int h):Widget(txt,w,h){
    setFlag(Attr::ATTR_FOCUSABLE);
    setFlag(Attr::ATTR_ENABLE);
    text_alignment_=DT_LEFT|DT_VCENTER;
    label_alignment_=DT_LEFT|DT_VCENTER;
    edit_mode_=0;
    label_width_=0;
    caret_pos_=0;
    labelBkColor=0;
    afterChanged=nullptr;
}

void EditBox::setTextWatcher(AfterTextChanged ls){
    afterChanged=ls;
}

void EditBox::setCaretPos(int idx){
    caret_pos_=idx; 
    invalidate(nullptr);
}
const std::string& EditBox::replace(size_t start,size_t len,const std::string&txt){
    text_.replace(start,len,txt);
    invalidate(nullptr);
    return text_;
}
const std::string& EditBox::replace(size_t start,size_t len,const char*txt,size_t size){
    text_.replace(start,len,txt,size);
    invalidate(nullptr);
    return text_;
}
void EditBox::setEditMode(int mode){
    edit_mode_=mode;
}

int EditBox::getCaretPos(){
    return caret_pos_;
}

void EditBox::setLabelColor(int c){
    labelBkColor=c;
    invalidate(nullptr);
}

void EditBox::setLabelWidth(int w){
    label_width_=w;
}

void EditBox::setLabel(const std::string&txt){
    label_=txt;
    invalidate(nullptr);
}

void EditBox::setLabelAlignment(int align){
    label_alignment_=align;
    invalidate(nullptr);
}

const std::string&EditBox::getLabel(){
    return label_;
}

bool EditBox::onKeyRelease(KeyEvent&evt){
    char ch;
    switch(evt.getKeyCode()){
    case NGL_KEY_LEFT:
        if(caret_pos_>0){
            setCaretPos(caret_pos_-1);
            return true;
        }break;
    case NGL_KEY_RIGHT:
        if(caret_pos_<text_.size()){
            setCaretPos(caret_pos_+1);
            return true;
        }break;
    case NGL_KEY_DOWN:
        if(caret_pos_<text_.size()){
           ch=text_[caret_pos_];
           if(ch>='0'&&ch<='9'){
              if(ch>'0')ch--;
              else if(ch=='0')ch='9';
              text_[caret_pos_]=ch;
              if(nullptr!=afterChanged)afterChanged(*this);
              invalidate(nullptr);
           }
           return true;
        }break;
    case NGL_KEY_UP:
        if(caret_pos_<text_.size()){
           ch=text_[caret_pos_];
           if(ch>='0'&&ch<='9'){
               if(ch<'9')ch++;
               else if(ch=='9')ch='0';
               text_[caret_pos_]=ch;
               if(nullptr!=afterChanged)afterChanged(*this);
               invalidate(nullptr);
           }
           return true; 
        }break;
    case NGL_KEY_BACKSPACE:
    case NGL_KEY_DEL:
    case NGL_KEY_0...NGL_KEY_9:
        ch='0'+(evt.getKeyCode()- NGL_KEY_0);
        if(edit_mode_==0||caret_pos_>=text_.size()){
           text_.insert(caret_pos_,1,ch);
           caret_pos_++;
        }else{
           text_[caret_pos_]=ch;
        }
        if(nullptr!=afterChanged)afterChanged(*this);
        invalidate(nullptr);
        return true;
    default:break; 
    }
    return false;
}

void EditBox::onDraw(GraphContext&canvas){
    RECT r=getClientRect();
    // Background
    canvas.set_color(bg_color_);
    canvas.draw_rect(r);
    // Border
    if(hasFlag(Attr::ATTR_BORDER)){
        canvas.set_color(fg_color_);
        canvas.set_line_width(1);
        canvas.rectangle(r);
        canvas.stroke();
    }
    
    canvas.set_color(fg_color_);
    canvas.set_font_size(getFontSize());
    if(label_width_>0){
        r.width=label_width_;
        if(!hasFlag(Attr::ATTR_FOCUSED)){
            canvas.set_color(labelBkColor);
            canvas.draw_rect(r);
        }
        r.inflate(-4,0);
        canvas.set_color(fg_color_);
        canvas.draw_text(r,label_,label_alignment_);
        r.x=label_width_;
        r.width=getWidth()-label_width_;
    }
    // Text
    canvas.draw_text(r,text_,text_alignment_);
    TextExtents te,te1;
    int caretx=label_width_;
    int caretw=getFontSize()/3;
 

    if(caret_pos_>=0 && text_.length()>0){
        std::string ss =text_.substr(0,caret_pos_);
        std::string ss2=text_.substr(0,caret_pos_+1);
        canvas.get_text_extents(ss,te);
        canvas.get_text_extents(ss2,te1);
        caretx=label_width_+te.width;
        caretw=(ss2.length()>ss.length())?(te1.width-te.width):(te1.width/ss2.length());
    }
    r.inflate(0,(te.height-r.height)/4); 
    if(hasFlag(Attr::ATTR_FOCUSED)){
        if(0==edit_mode_){
            canvas.move_to(caretx,r.y);
            canvas.line_to(caretx,r.y+r.height);
        }else{
            //canvas.move_to(caretx,r.y+r.height);
            //canvas.line_to(caretx+caretw,r.y+r.height);
            canvas.rectangle(caretx,r.y,caretw,r.height);
        }
        canvas.stroke();
    }
}
}


