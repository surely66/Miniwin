#include<editbox.h>
#include<keyboard.h>
#include<ngl_ir.h>
#include<ngl_log.h>
#include <regex>

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
    caretPos=0;
    labelBkColor=0;
    afterChanged=nullptr;
}

void EditBox::setTextWatcher(AfterTextChanged ls){
    afterChanged=ls;
}

void EditBox::setCaretPos(int idx){
    caretPos=idx; 
    invalidate(nullptr);
}

void EditBox::setPattern(const std::string&pattern){
    inputPattern=pattern;
}

bool EditBox::match(){
    if(inputPattern.empty())
       return true;
    std::regex reg(inputPattern);
    return std::regex_match(text_,reg);
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
    return caretPos;
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

int EditBox::Unicode2UTF8(UINT uni,std::string&str){
   if(uni<=0x7F){
       str+=uni;
       return 1;
   }else if(uni>=0x80&&uni<=0x7FF){
       str+=0xC0|((uni>6)&0x1F);
       str+=0x80|(uni&0x3F);
       return 2; 
   }else if(uni>=0x800&&uni<=0xFFFF){
       str+=0xE0|(uni>>12);
       str+=0x80|((uni>>6)&0x3F);
       str+=0x80|(uni&0x3F);
       return 3;
   }else if(uni>=0x10000){
       str+=0xF0|((uni>>18)&0x07);
       str+=0x80|((uni>>12)&0x3F);
       str+=0x80|((uni>>6)&0x3F);
       str+=0x80|(uni&0x3F);
       return 4;
   }
   return 0;
}

bool EditBox::onMessage(DWORD msg,DWORD wp,ULONG lp){
    if(msg!=WM_CHAR)
        return Widget::onMessage(msg,wp,lp);
    std::string txt;
    Unicode2UTF8(wp,txt);
    if(edit_mode_==0||caretPos>=text_.size()){
         text_.insert(caretPos,txt);
         caretPos++;
    }else{
         text_.replace(caretPos,1,txt);
    }
    if(nullptr!=afterChanged)
            afterChanged(*this);
    invalidate(nullptr);
    return true;
}

bool EditBox::onKeyRelease(KeyEvent&evt){
    char ch;
    bool ret=false;
    int changed=0;
    std::string text_old=text_;
    INT caretpos_old=caretPos;
    switch(evt.getKeyCode()){
    case NGL_KEY_LEFT:
        if(caretPos>0){
            setCaretPos(caretPos-1);
            return true;
        }break;
    case NGL_KEY_RIGHT:
        if(caretPos<text_.size()){
            setCaretPos(caretPos+1);
            return true;
        }break;
    case NGL_KEY_DOWN:
        if(caretPos<text_.size()){
           ch=text_[caretPos];
           if(ch>='0'&&ch<='9'){
               if(ch>'0')ch--;
               else if(ch=='0')ch='9';
               text_[caretPos]=ch;
               changed++;
               ret=true;
           }
        }break;
    case NGL_KEY_UP:
        if(caretPos<text_.size()){
           ch=text_[caretPos];
           if(ch>='0'&&ch<='9'){
               if(ch<'9')ch++;
               else if(ch=='9')ch='0';
               text_[caretPos]=ch;
               changed++;
               ret=true;
           }
        }break;
    case NGL_KEY_BACKSPACE:
        if(caretPos>0){
             text_.erase(caretPos-1,1);
             changed++;
             ret=true;
        }break;
    case NGL_KEY_DEL:
        if(caretPos<text_.size()-1){
            text_.erase(caretPos,1);
            changed++;
            ret=true; 
        }break;
    case NGL_KEY_0...NGL_KEY_9:
        sendMessage(WM_CHAR,'0'+(evt.getKeyCode()- NGL_KEY_0),0);
        return true;
    case NGL_KEY_MENU:
        {
            Keyboard *kb=new Keyboard(320,400,640,240);
            kb->setBuddy(this);
        }return true;
    default:break; 
    }
    if(changed){
        if(!match()){
             text_=text_old;
             caretPos=caretpos_old;
             return false;
        }  
        invalidate(nullptr);
        if(nullptr!=afterChanged)
            afterChanged(*this);
    }
    return ret;
}

void EditBox::onDraw(GraphContext&canvas){
    Widget::onDraw(canvas);
    RECT r=getClientRect();
    
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
 

    if(caretPos>=0 && text_.length()>0){
        std::string ss =text_.substr(0,caretPos);
        std::string ss2=text_.substr(0,caretPos+1);
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


