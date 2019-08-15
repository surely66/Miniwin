#include<listview.h>
#include <ngl_log.h>
#include <ngl_ir.h>
NGL_MODULE(LISTVIEW)

namespace nglui{

ListView::ListView(int w,int h)
 :INHERITED(w,h){
}

bool ListView::onKeyRelease(KeyEvent&k){
    int pagesize,itmheight,cnt=getItemCount();
    int idx=getIndex();
    ListItem*itm=getItem(idx);
    if(0==cnt)return false;
    list_[0]->onGetSize(*this,nullptr,&itmheight);
    pagesize=getHeight()/itmheight;
    switch(k.getKeyCode()){
    case NGL_KEY_UP:  
         if(idx==top_&&idx>0){
             top_--;
             invalidate(nullptr);
         }
         setIndex(idx>0?idx-1:0);
         break;
    case NGL_KEY_DOWN:
         if(itm&&itm->rect.y+itm->rect.height>=getHeight()){
             top_++;
             invalidate(nullptr);
         }
         if(idx<(int)(list_.size()-1))setIndex(idx+1);
         break;
    case NGL_KEY_PGUP:
         top_=top_>pagesize?top_-pagesize:0;
         idx=index_>pagesize?index_-pagesize:0;
         INHERITED::setIndex(idx);
         invalidate(nullptr);
         return true;
    case NGL_KEY_PGDOWN:
         top_=top_+pagesize<cnt?top_+pagesize:cnt-pagesize;
         idx=index_+pagesize<cnt?index_+pagesize:cnt-1;
         INHERITED::setIndex(idx);
         invalidate(nullptr);
         return true;
    case NGL_KEY_ENTER:
         return View::onKeyRelease(k);
    default:return false;
    }
    return true;
}

void ListView::setIndex(int idx){
    ListItem*itm=getItem(index_);
    if(itm)invalidate(&itm->rect);
    itm=getItem(idx);
    if(itm)invalidate(&itm->rect);
    if(idx>=0&&idx<list_.size())
       INHERITED::setIndex(idx);
}

void ListView::onDraw(GraphContext&canvas){
    RECT rect=getClientRect();
    canvas.set_color(getBgColor());
    canvas.draw_rect(rect);
    RECT r;
    r.set(0,0,getWidth(),0);
    canvas.set_font_size(getFontSize());
    int idx=top_;
    r.height+=8;
    r.inflate(-2,0);
    if(list_.size()>0)
      list_[0]->onGetSize(*this,nullptr,&r.height);
    for(auto itm=list_.begin()+idx;itm<list_.end();itm++,idx++){
        (*itm)->rect=r;
        item_painter_(*this,*(*itm),idx==index_,canvas);
        r.offset(0,r.height);
        if(r.y>getHeight())break;
    }
    SCROLLINFO sb;
    sb.fMask=SIF_PAGE|SIF_RANGE|SIF_POS;
    sb.nPage=idx-top_;
    sb.nMin=0;
    sb.nMax=list_.size();
    sb.nPos=top_;
    if(hasFlag(Attr::ATTR_SCROLL_VERT)&&sb.nMax>sb.nPage){ 
       setScrollInfo(SB_VERT,&sb);
       drawScrollBar(canvas,SB_VERT);
    }
    if(hasFlag(Attr::ATTR_BORDER)){
        canvas.set_color(getFgColor());
        canvas.rectangle(getClientRect());
        canvas.stroke();
    }
}

}//namespace:
