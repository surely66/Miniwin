#include<selector.h>
#include <ngl_ir.h>
#include <ngl_log.h>
#include <windows.h>

NGL_MODULE(SELECTOR)

namespace nglui{

Selector::Selector(int w,int h)
  :Selector(std::string(),w,h){
}

Selector::Selector(const std::string&txt,int w,int h)
  :INHERITED(txt,w,h){
   label_width_=0;
   popupRect.set(0,0,0,0);
   show_arrows_=SHOW_ONFOCUSED;
   labelBkColor=0;
   onpopwin=nullptr;
}

void Selector::showArrows(int mode){
    show_arrows_=mode;
}

void Selector::setLabelColor(int color){
    labelBkColor=color;
}

void Selector::setLabelWidth(int w){
   if(label_width_!=w){
       label_width_=w;
       invalidate(nullptr);
   }
}

int Selector::getLabelWidth(){
    return label_width_;
}

void Selector::setPopupListener(CreatePopupListener ls){
    onpopwin=ls;
}

void Selector::setPopupRect(int x,int y,int w,int h){
    RECT r;
    r.set(x,y,w,h);
    setPopupRect(r);
}

class PopWindow:public Window{
public:
    PopWindow(int x,int y,int w,int h):Window(x,y,w,h){}
    virtual bool onKeyRelease(KeyEvent&kv)override{
        if(Window::onKeyRelease(kv)==false && kv.getKeyCode()==NGL_KEY_ENTER){
             sendMessage(WM_DESTROY,0,0);
             return true;
        }
    }
};

Window*Selector::createPopupWindow(){
    Window*wpop;
    ListView*lv;
    if(onpopwin==nullptr){
        wpop=new PopWindow(popupRect.x,popupRect.y,popupRect.width,popupRect.height);
        lv=new ListView(popupRect.width,popupRect.height);
        wpop->addChildView(lv);
    }else{
        wpop=onpopwin(popupRect.width,popupRect.height);
        wpop->setPos(popupRect.x,popupRect.y);
        lv=(ListView*)wpop->getChildView(0);
    }
    if(parent_!=nullptr){
        wpop->setBgColor(parent_->getBgColor());
        wpop->setFgColor(parent_->getFgColor());
    }
    
    lv->setItemSelectListener([this](AbsListView&lv,int idx){
        setIndex(idx);
    });
    for(int i=0;i<getItemCount();i++){
        lv->addItem(list_[i]);
    }
    lv->setIndex(getIndex());
    return wpop;
}

void Selector::setPopupRect(const RECT&rect){
    popupRect=rect;
}

void Selector::setIndex(int idx){
    INHERITED::setIndex(idx);
    invalidate(nullptr);
}

bool  Selector::onKeyRelease(KeyEvent&k){
    int cnt=getItemCount();
    if(0==cnt)
         return false;
    switch(k.getKeyCode()){
    case NGL_KEY_LEFT:setIndex((getIndex()-1+cnt)%cnt);
          return true;
    case NGL_KEY_RIGHT:setIndex((getIndex()+1)%cnt);
          return true;
    case NGL_KEY_ENTER:
          if(!popupRect.empty()){
               createPopupWindow();
               return true;
          }break;
    default:return false;
    }
    return false;
}

void Selector::onDraw(GraphContext&canvas){
    const static int MARGIN = 5;
    RECT rect=getClientRect();
    if(hasFlag(Attr::ATTR_FOCUSED)){
        canvas.set_color(0xFF008000);
        canvas.draw_rect(rect);
    }else{
        if(labelBkColor!=0&&label_width_>0){
            rect.width=label_width_;
            canvas.set_color(labelBkColor);
            canvas.draw_rect(rect);
            rect.x=label_width_;
            rect.width=getWidth()-label_width_;
        }
        canvas.set_color(getBgColor());
        canvas.draw_rect(rect);
    }
    if(hasFlag(Attr::ATTR_BORDER)){
        canvas.set_color(getFgColor());
        canvas.rectangle(rect);
        canvas.stroke();
    }

    int alignment_=DT_LEFT|DT_VCENTER;
    canvas.set_font_size(getFontSize());
    canvas.set_color(getFgColor());    
    if(getLabelWidth()){
        rect.set(MARGIN,0,getLabelWidth()-MARGIN-MARGIN,getHeight());
        canvas.draw_text(rect,getText(),alignment_);
    }
    ListItem*itm=getItem(getIndex());
    rect.set(getLabelWidth(),0,getWidth()-getLabelWidth(),getHeight());
    if(itm!=nullptr){
        canvas.draw_text(rect,itm->getText(),DT_CENTER|DT_VCENTER);
    }
    if( (show_arrows_==SHOW_ALWAYS) ||((show_arrows_==SHOW_ONFOCUSED)&&hasFlag(Attr::ATTR_FOCUSED))){
        rect.inflate(-MARGIN,-MARGIN); 
        canvas.draw_arraw(rect,2);
        canvas.draw_arraw(rect,3);
    }
}
    
}//namespace

