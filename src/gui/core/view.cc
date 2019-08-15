#include <view.h>
#include <ngl_log.h>
#include <ngl_ir.h>

NGL_MODULE(VIEW)

namespace nglui{

View::View(int w,int h)
  : id_(0),  parent_(nullptr),
    fg_color_(DefaultFgColor),
    bg_color_(DefaultBgColor),
    attr_(Attr::ATTR_VISIBLE),
    font_size_(DefaultFontSize){
   layout_.reset(nullptr);
   bound_.set(0,0,w,h);
   prefer_size_.set(w,h);
   invalid_region_=cairo_region_create();
   invalidate(nullptr);
   onclick_=nullptr;
   onmessage_=nullptr;
   memset(&scrollinfos,0,sizeof(SCROLLINFO)*2);
}

View::~View(){
    children_.clear();
    cairo_region_destroy(invalid_region_);
}

View*View::findViewById(int id){
    for(auto c=children_.begin();c!=children_.end();c++){
         if((*c)->children_.size()){
            auto cc=(*c)->findViewById(id);
            if(cc)return cc;
         }
         if((*c)->id_==id)
            return c->get();
    }
    return nullptr;
}

void View::setScrollInfo(int bar,const SCROLLINFO*info,bool redraw){
    if(info==nullptr||bar<0||bar>1)return;
    if(info->fMask&SIF_RANGE){
        scrollinfos[bar].nMin=info->nMin;
        scrollinfos[bar].nMax=info->nMax;
    }if(info->fMask&SIF_PAGE){
        scrollinfos[bar].nPage=info->nPage;
    }if(info->fMask&SIF_POS){
        scrollinfos[bar].nPos=info->nPos;
    }if(info->fMask&SIF_TRACKPOS){
        scrollinfos[bar].nTrackPos=info->nTrackPos;
    }
}

void View::drawScrollBar(GraphContext&canvas,int bar){
    RECT rc=getClientRect();
    SCROLLINFO sb=scrollinfos[bar];
    rc.inflate(-1,-1);
    int barsize=(bar==SB_VERT)?rc.height:rc.width;
    int barpos=barsize*(sb.nPos)/(sb.nMax-sb.nMin+1);
    barsize=barsize*sb.nPage/(sb.nMax-sb.nMin+1);
    NGLOG_VERBOSE("BARINFO range=%d,%d pos=%d page=%d",sb.nMin,sb.nMax,sb.nPos,sb.nPage);
    NGLOG_VERBOSE("barsize=%d pos=%d view.size=%d,%d ",barsize,barpos,rc.width,rc.height);
    if(bar==SB_VERT){
        rc.set(rc.width-8,rc.y,8,rc.height);
        RECT rb=rc;rb.inflate(-3,0);
        canvas.set_color(0xFF222222);
        canvas.draw_rect(rb);
        canvas.set_color(0xFF888888);
        rc.height=barsize;
        rc.y=barpos;
        canvas.draw_rect(rc);
    }else{
        rc.set(rc.x,rc.height-10,rc.width,10);
        canvas.set_color(0xFF888888);
        canvas.draw_rect(rc);
        canvas.set_color(0xFFFF0000);
        rc.width=barsize;
        rc.x=barpos;
        canvas.draw_rect(rc);
    }
}

void View::setClickListener(ClickListener ls){
    onclick_=ls;
}

void View::setMessageListener(MessageListener ls){
    onmessage_=ls;
}
void View::clip(GraphContext&canvas){
    canvas.reset_clip();
    for(auto c:children_){
        RECT rc=c->getBound();
        cairo_rectangle_int_t r={rc.x,rc.y,rc.width,rc.height};
        if((c->isVisible()==false)||(c->hasFlag(Attr::ATTR_TRANSPARENT)&&c->getChildrenCount()==0))continue;
        cairo_region_subtract_rectangle(invalid_region_,&r);
    }
    int num=cairo_region_num_rectangles(invalid_region_);
    for(int i=0;i<num;i++){
        cairo_rectangle_int_t r;
        cairo_region_get_rectangle(invalid_region_,i,&r);
        canvas.rectangle(r.x,r.y,r.width,r.height);
    }
    canvas.clip();
}

void View::resetClip(){
    cairo_rectangle_int_t r={0,0,0,0};
    cairo_region_intersect_rectangle(invalid_region_,&r);
}

void View::onDraw(GraphContext&canvas){
    canvas.set_color(getBgColor());
    RECT r=getClientRect();
    canvas.draw_rect(r);
}

View::AnimState View::getAnimateState() {
    return anim_state_;
}

void View::setAnimateState(AnimState state) {
    anim_state_ = state;
}

void View::onAnimate() {
}

const RECT& View::getBound(){
    return bound_;
}

View& View::setId(int id){
    id_=id;
    return *this;
}

int View::getId() const{
    return id_;
}

int View::getWidth(){
    return bound_.width;
}

int View::getHeight(){
    return bound_.height;
}

int View::getX(){
    return bound_.x;
}

int View::getY(){
    return bound_.y;
}

View& View::setBound(const RECT&b){
    bound_=b;
    invalidate(nullptr);
    return *this;
}

View& View::setBound(int x,int y,int w,int h){
    bound_.set(x,y,w,h);
    invalidate(nullptr);
    return *this;
}

View& View::setPos(int x,int y){
    if(parent_)
       parent_->invalidate(&bound_);
    bound_.x=x;
    bound_.y=y; 
    invalidate(nullptr);
    return *this;
}

View& View::setSize(int w,int h){
    bound_.width=w;
    bound_.height=h;
    invalidate(nullptr);
    return *this;
}

const RECT View::getClientRect(){
    RECT r=bound_;
    r.x=r.y=0;
    return r;
}

View& View::setFgColor(UINT color){
    fg_color_=color;
    return *this;
}

UINT View::getFgColor() const{
    return fg_color_;
}

View& View::setBgColor(UINT color){
    bg_color_=color;
    return *this;
}

UINT View::getBgColor() const{
    return bg_color_;
}

const SIZE&View::getPreferSize(){
    prefer_size_.set(getWidth(),getHeight());
    return prefer_size_;
}

void View::setFlag(Attr flag) {
    attr_ = static_cast<Attr>(static_cast<uint32_t>(attr_) | static_cast<uint32_t>(flag));
}

void View::clearFlag(Attr flag) {
    attr_ = static_cast<Attr>(static_cast<uint32_t>(attr_) & ~static_cast<uint32_t>(flag));
}

void View::resetFlag() {
    attr_ = Attr::ATTR_NONE;
}

bool View::hasFlag(Attr flag) const {
    if (static_cast<uint32_t>(attr_) & static_cast<uint32_t>(flag)) {
       return true;
    }
    return false;
}

bool View::isFocused()const {
   return hasFlag(Attr::ATTR_FOCUSED);
}

void View::setVisible(bool visable) {
   if(visable)
        setFlag(Attr::ATTR_VISIBLE);
   else 
        clearFlag(Attr::ATTR_VISIBLE);
}

bool View::isVisible() const {
   return hasFlag(Attr::ATTR_VISIBLE);
}

void View::setEnable(bool enable) {
    if(enable)
        setFlag(Attr::ATTR_ENABLE);
    else 
        clearFlag(Attr::ATTR_ENABLE);
}

bool View::isEnable() const {
    return hasFlag(Attr::ATTR_ENABLE);
}

int View::getFontSize() const{
    return font_size_;
}

View& View::setFontSize(int sz){
    font_size_=sz;
    return *this;
}

void View::invalidate(const RECT*rect){
#if 1
    cairo_rectangle_int_t r;
    RECT rc=getClientRect();
    if(rect && rect->empty()==false)
         rc=*rect;
    r.x=rc.x;    r.y=rc.y;
    r.width= rc.width;
    r.height=rc.height;
     
    cairo_region_union_rectangle(invalid_region_,&r);
    if(parent_){
       rc.offset(getX(),getY());
       parent_->invalidate(&rc);
    }
#else
    DWORD wp=0,lp=0;
    if(rect){
         wp=rect->x<<16|rect->y;
         lp=rect->width<<16|rect->height;
    }
    sendMessage(WM_INVALIDATE,wp,lp);
#endif
}

void View::invalidate_inner(const RECT*rect){
    cairo_rectangle_int_t r;
    RECT rc=getClientRect();
    if(rect)
         rc=*rect;
    r.x=rc.x;    r.y=rc.y;
    r.width= rc.width;
    r.height=rc.height;

    cairo_region_union_rectangle(invalid_region_,&r);
    if(parent_){
       rc.offset(getX(),getY());
       parent_->invalidate(&rc);
    }
}
// Layout
void View::setLayout(Layout* layout){
    layout_.reset(layout);
    if(children_.size())
        onLayout();
}


Layout* View::getLayout() const{
     return layout_.get();
}

void View::onLayout()
{
    if (isVisible() && (layout_.get() != nullptr)) {
        layout_->onLayout(this);
    }
}

View*View::getParent(){
    return parent_;
}

void View::setParent(View*p){
    parent_=p;
}

int View::getViewOrder(View*v){
    int idx=0;
    for(auto it=children_.begin();it!=children_.end();it++,idx++){
        if(it->get()==v)return idx;
    }
    return -1;
}

View* View::getChildView(size_t idx){
    if (idx >= children_.size()) {
        return nullptr;
    }
    return (View*)children_[idx].get();
}

View* View::addChildView(View* view){
    if(nullptr==view)
         return nullptr;
    view->setParent(this);
    std::shared_ptr<View>itm;itm.reset(view);
    children_.push_back(itm);
    onLayout();
    return itm.get();
}


void View::removeChildView(View* view){
    for (std::vector< std::shared_ptr<View>>::iterator it = children_.begin();
          it != children_.end();++it) {
        if ((*it).get() == view) {
            children_.erase(it);
            onLayout();
            return ;
        }
    }
}

void View::removeChildView(size_t idx){
    View* pView = getChildView(idx);
    removeChildView(pView);     
}

size_t View::getChildrenCount() const{
    return children_.size();
}

void View::onResize(SIZE& size){
}

bool View::onKeyChar(KeyEvent& evt){
    return false;
}

bool View::onKeyPress(KeyEvent& evt){
    return false;
}

bool View::onKeyRelease(KeyEvent& evt){
    if(evt.getKeyCode()==NGL_KEY_ENTER && onclick_){
        onclick_(*this);
        return true;
    }
    return false;
}
bool View::onMousePress(Event& evt){
    return false;
}

bool View::onMouseRelease(Event& evt){
    return false;
}

bool View::onMouseMotion(Event& evt){
    return false;
}

void View::sendMessage(DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime){
    if(parent_){
        for(auto c=parent_->children_.begin();c!=parent_->children_.end();c++){
              if((*c).get()==this){
                 parent_->sendMessage((*c),msgid,wParam,lParam,delayedtime);
                 return;
              }
        }
    }
}

void View::sendMessage(std::shared_ptr<View>w,DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime){
    if(parent_)
       parent_->sendMessage(w,msgid,wParam,lParam,delayedtime);
}

bool View::onMessage(DWORD msgid,DWORD wParam,ULONG lParam){
    RECT r;
    switch(msgid){
    case WM_INVALIDATE://printf("View::onMessage WM_INVALIDATE %p\r\n",this);
         invalidate_inner(nullptr);
         r.set(wParam>>16,wParam&0xFFFF,lParam>>16,lParam&0xFFFF);
         invalidate( (0==wParam&&0==lParam)?nullptr:&r );
         return true;
    default:
         if(onmessage_)onmessage_(*this,msgid,wParam,lParam);
         return false;   
    }
}

}//endof namespace
