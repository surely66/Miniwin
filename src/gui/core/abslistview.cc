#include <abslistview.h>
#include <ngl_log.h>
#include <algorithm>
NGL_MODULE(ABSLISTVIEW)

namespace nglui{

AbsListView::AbsListView(int w,int h)
:AbsListView(std::string(),w,h){
}

AbsListView::AbsListView(const std::string& txt,int w,int h)
:INHERITED(txt,w,h){
   setBgColor(DefaultBgColor);
   setFgColor(DefaultFgColor);
   setFlag(Attr::ATTR_FOCUSABLE);
   setFlag(Attr::ATTR_ENABLE);
   index_=-1;
   top_=0;
   item_select_listener_=nullptr;
   item_painter_=DefaultPainter;// nullptr;
}

void AbsListView::DefaultPainter(AbsListView&lv,const ListItem&itm,int state,GraphContext&canvas){
    canvas.set_color(state?0xFFFF0000:lv.getBgColor());
    canvas.draw_rect(itm.rect);
    canvas.set_color(lv.getFgColor());
    canvas.draw_text(itm.rect,itm.getText(),DT_LEFT|DT_VCENTER);
}

void AbsListView::setItemPainter(ItemPainter painter){
    item_painter_=painter;
}

void AbsListView::setItemSelectListener( ItemSelectListener listener){
    item_select_listener_=listener;    
}

void AbsListView::sort(ItemCompare cmp,bool revert){
#if 1//std::sort myb caused crash :( if function cmp has logical error.
     std::sort(list_.begin(),list_.end(),
          [&](std::shared_ptr<ListItem>a, std::shared_ptr<ListItem> b)->bool{
                 NGLOG_DEBUG_IF(a==nullptr||b==nullptr,"a=%p b=%p",a,b);
                 bool rc=cmp(*a,*b);
                 return (revert==false)?rc:(!rc);
          });
#else //popsort 
    for(size_t i=0;i<list_.size();i++){
        for(size_t j=i+1;j<list_.size();j++){
           if( ((revert==false)&&(cmp(*list_[i],*list_[j])>0))
               ||(revert && (cmp(*list_[i],*list_[j])<0)) ){
              std::shared_ptr<ListItem>t=list_[i];
              list_[i]=list_[j];
              list_[j]=t;  
           }
        }
    }
#endif
}

void AbsListView::setIndex(int idx){
    if( index_!=idx && idx>=0 && idx<list_.size() ){
        index_=idx;
        if(item_select_listener_!=nullptr)
            item_select_listener_(*this,index_);
    }
}

void AbsListView::setTop(int idx){
    if(top_!=idx){
         top_=idx; 
         invalidate(nullptr);
    }
}

int AbsListView::getIndex(){
    return index_;
}

int AbsListView::getTop(){
    return top_;
}

unsigned int AbsListView::getItemCount(){
    return list_.size();
}

AbsListView::ListItem* AbsListView::getItem(int idx){
    if(idx>=0&&idx<list_.size()){
        return list_[idx].get();
    }
    return nullptr;
}

void AbsListView::addItem(AbsListView::ListItem*item){
    std::shared_ptr<AbsListView::ListItem>p(item);
    addItem(p);
}

void AbsListView::addItem(std::shared_ptr<ListItem>itm){
    list_.push_back(itm);
    invalidate(nullptr); 
}

void AbsListView::removeItem(int idx){
    if(idx==index_){
        if(idx==0)index_==0;
        if(idx==list_.size()-1);index_=list_.size()-2;//do nothingindex_--;redraw++;
        if(item_select_listener_!=nullptr)
            item_select_listener_(*this,index_);
    }else if(index_>idx){//idx<=index_
        if(idx>0)index_--;
    }
    if(idx<top_&&idx>0){top_--;}
    list_.erase(list_.begin()+idx);
    invalidate(nullptr);
}

void AbsListView::removeItem(AbsListView::ListItem*itm){
    size_t idx=0;
    for(auto itr=list_.begin();itr!=list_.end();itr++,idx++){
        if(itr->get()==itm){
           removeItem(idx);//list_.erase(itr);
           break;
        }
    }
}

void AbsListView::clearAllItems(){
    list_.erase(list_.begin(),list_.end());
    index_=-1;top_=0;
    invalidate(nullptr);
}


AbsListView::ListItem::ListItem(const std::string&txt,int v)
  :text_(txt){
  value_=v;
}

AbsListView::ListItem::~ListItem(){
}

const std::string& AbsListView::ListItem::getText()const{
   return text_;
}

void AbsListView::ListItem::setText(const std::string&txt){
   text_=txt;   
}

void AbsListView::ListItem::onGetSize(AbsListView&lv,int* w,int* h){
   int tw,th;
   if(w)*w=(int)lv.getWidth();
   if(h)*h=(int)lv.getFontSize()*3/2;
}

}//namespace
