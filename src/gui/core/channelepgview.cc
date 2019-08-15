#include<channelepgview.h>
#include<ngl_timer.h>
#include <ngl_log.h>

NGL_MODULE(CHANNELEPG)
namespace nglui{

void ChannelBar::addEvent(const TVEvent&evt){
     events.push_back(evt);
}

void ChannelBar::addEvent(const std::string name,ULONG starttm,ULONG dur){
     TVEvent e;
     e.name=name;
     e.start=starttm;
     e.duration=dur;
     addEvent(e);
}

void ChannelBar::clearEvents(){
     events.clear();
}

void ChannelEpgView::DefaultPainter(AbsListView&lv,const AbsListView::ListItem&itm,int state,GraphContext&canvas){
     int idx=0;
     ULONG starttime=((ChannelEpgView&)lv).getStartTime();
     float pixelmin =((ChannelEpgView&)lv).getPixelPerMinute();
     int nameWidth=((ChannelEpgView&)lv).getChannelNameWidth();
     ChannelBar&bar=(ChannelBar&)itm;
     canvas.set_font_size(16);
     for(auto e:bar.events){
         RECT r=bar.rect;
         r.x=((int)(e.start-starttime))/60*pixelmin+nameWidth;
         r.width=e.duration/60*pixelmin;
         canvas.set_color(0xFF888888);
         r.height-=1;
         r.width-=1;
         canvas.draw_rect(r);
         canvas.set_color(0xFFFF0000);//lv.getFgColor());
         canvas.draw_text(r,e.name);
     }
     RECT r=bar.rect;
     canvas.set_color(state?0xFFFF0000:0xFF444444);
     r.width=nameWidth-1;
     r.height-=1;
     canvas.draw_rect(r);
     canvas.set_color(0xFF000000); 
     canvas.draw_text(r,bar.getText());
}

void ChannelBar::onGetSize(AbsListView&lv,int*w,int*h){
     if(w)*w=lv.getWidth();
     if(h)*h=40;
}

////////////////////////////////////////////////////////////////////////////////////////////////

static void Time2Hour(ULONG *start){
     NGL_TM t;
     nglTimeToTm(start,&t);
     t.uiMin=0;
     t.uiSec=0;
     nglTmToTime(&t,start);
}

ChannelEpgView::ChannelEpgView(int w,int h):ListView(w,h){
     pixelMinute=1.0;
     nglGetTime(&starttime);
     Time2Hour(&starttime);
     nameWidth=120;
     timeRuleHeight=12;
     item_painter_=ChannelEpgView::DefaultPainter;
}

void ChannelEpgView::setStartTime(ULONG t){
     starttime=t;
     Time2Hour(&starttime);
     invalidate(nullptr);
}

ULONG ChannelEpgView::getStartTime(){
     return starttime;
}

void ChannelEpgView::setPixelPerMinute(float p){
     pixelMinute=p;
}

void ChannelEpgView::setChannelNameWidth(int w){
     nameWidth=w;
}

int ChannelEpgView::getChannelNameWidth(){
     return nameWidth;
}

void ChannelEpgView::setTimeRuleHeight(int h){
     timeRuleHeight=h;
}

int ChannelEpgView::getTimeRuleHeight(){
     return timeRuleHeight;
}

float ChannelEpgView::getPixelPerMinute(){
     return pixelMinute;
}

void ChannelEpgView::drawTimeRule(GraphContext&canvas){
    ULONG st=starttime;
    canvas.set_color(0xFF000000);
    std::string ts;
    int y,x=nameWidth;
    NGL_TM t;
    canvas.set_font_size(12);
    nglTimeToTm(&st,&t);
    TextExtents ext;
    ts=std::to_string(t.uiMonth)+"/"+std::to_string(t.uiMonthDay)+"/"+std::to_string(1900+t.uiYear);
    canvas.get_text_extents(ts,ext);
    y=(timeRuleHeight-ext.height)/2-ext.y_bearing;
    canvas.move_to((nameWidth-ext.width)/2,y);
    canvas.text_path(ts);
    for(;x<getWidth();){
        x=(st-starttime)/60*pixelMinute+nameWidth;
        canvas.move_to(x,0);
        canvas.line_to(x,timeRuleHeight);
        st+=3600;
        canvas.move_to(x+1,y);
        nglTimeToTm(&st,&t);
        ts=std::to_string(t.uiHour)+":"+std::to_string(t.uiMin);
        canvas.text_path(ts);
    }
    canvas.move_to(0,timeRuleHeight-1);
    canvas.line_to(getWidth(),timeRuleHeight-1);
    canvas.stroke();
    canvas.set_font_size(getFontSize());
}

void ChannelEpgView::onDraw(GraphContext&canvas){
    RECT rect=getClientRect();
    canvas.set_color(getBgColor());
    canvas.draw_rect(rect);

    canvas.set_color(getFgColor());
    canvas.rectangle(rect);
    canvas.stroke();

    RECT r;
    r.set(0,timeRuleHeight,getWidth(),0);
    canvas.set_font_size(getFontSize());
    int idx=top_;
    r.height+=8;
    r.inflate(-2,0);
    if(list_.size()>0)
      list_[0]->onGetSize(*this,nullptr,&r.height);
    drawTimeRule(canvas);
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
    ULONG timenow;
    nglGetTime(&timenow);
    int xx=(timenow-starttime)/60*pixelMinute+nameWidth;
    canvas.set_color(0xFFFF0000);
    canvas.move_to(xx,timeRuleHeight+2);
    canvas.line_to(xx,getHeight()-2);
    canvas.stroke();
}

}//end namespace
