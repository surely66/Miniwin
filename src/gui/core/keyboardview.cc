#include<keyboardview.h>
#include<ngl_log.h>
#include<ngl_ir.h>

NGL_MODULE(KEYBOARD)

namespace nglui{


KeyboardView::KeyboardView(int w,int h):View(w,h){
    key_bg_color=0xFF222222;
    kx=ky=0;
    kcol=krow=0;
    key_index=0;
    setFlag(Attr::ATTR_FOCUSABLE);
    setFlag(Attr::ATTR_ENABLE);
    kbd_lines.push_back(std::vector<int>());
}

void KeyboardView::addKey(UINT code,UINT uper,USHORT w,USHORT h){
    KEY k;
    k.unicode=code;
    k.upcode=uper;
    k.x=kx;
    k.y=ky;
    k.width=w;
    k.height=h;
    if(KC_NONE==code){
         if(0==w){
            ky+=h;
            kbd_lines.push_back(std::vector<int>());
            kx=0;
         }else{
             kx+=w;
         }
    }else{   
         kbd_lines.back().push_back(keys.size()); 
         keys.push_back(k);
         kx+=w;
    }
}

void KeyboardView::setKeyBgColor(UINT cl){
    key_bg_color=cl;
}

UINT KeyboardView::getKeyBgColor(){
    return key_bg_color;
}

static void Unicode2UTF8(UINT uni,std::string&str){
   if(uni<=0x7F){
      str+=uni;
   }else if(uni>=0x80&&uni<=0x7FF){
      str+=0xC0|((uni>6)&0x1F);
      str+=0x80|(uni&0x3F);
   }else if(uni>=0x800&&uni<=0xFFFF){
      str+=0xE0|(uni>>12);
      str+=0x80|((uni>>6)&0x3F);
      str+=0x80|(uni&0x3F);
   }else if(uni>=0x10000){
      str+=0xF0|((uni>>18)&0x07);
      str+=0x80|((uni>>12)&0x3F);
      str+=0x80|((uni>>6)&0x3F);
      str+=0x80|(uni&0x3F);
   }
}

void KeyboardView::setKeyIndex(UINT idx){
    if(key_index!=idx){
        key_index=idx;
        invalidate(NULL);
    }
}
void KeyboardView::onDraw(GraphContext&canvas){

    View::onDraw(canvas);
    canvas.set_color(getKeyBgColor());
    for(int i=0;i<keys.size();i++){
        KEY&k=keys[i];
        canvas.rectangle(k.x,k.y,k.width,k.height);
    }    
    canvas.fill();
    if(key_index>=0&&key_index<keys.size()){
        canvas.set_color(0xFF00FF00); 
        KEY&k=keys[key_index];
        canvas.rectangle(k.x,k.y,k.width,k.height);
        canvas.fill();
    }
    canvas.set_color(getFgColor());
    canvas.set_font_size(getFontSize());
    for(int i=0;i<keys.size();i++){
         KEY&k=keys[i];
         RECT r={k.x,k.y,k.width,k.height};
         std::string txt;
         Unicode2UTF8(k.unicode,txt);
         canvas.draw_text(r,txt,DT_VCENTER|DT_CENTER);
    }
    canvas.fill();
}

bool KeyboardView::onKeyRelease(KeyEvent&k){
    switch(k.getKeyCode()){
    case NGL_KEY_LEFT: 
                       kcol=(kcol-1+kbd_lines[krow].size())%kbd_lines[krow].size();
                       break;
    case NGL_KEY_RIGHT:
                       kcol=(kcol+1)%kbd_lines[krow].size();
                       break;
    case NGL_KEY_UP:   
                       krow=(krow-1)%kbd_lines.size();
                       if(kbd_lines[krow].size()<kcol)
                           kcol=kbd_lines[krow].size()-1;
                       break;
    case NGL_KEY_DOWN: krow=(krow+1)%kbd_lines.size();
                       if(kbd_lines[krow].size()<kcol)
                           kcol=kbd_lines[krow].size()-1;      
                       break;
    case NGL_KEY_ENTER:
    default: break;
    }
    setKeyIndex(kbd_lines[krow][kcol]);
    return false;
}

}//namespace
