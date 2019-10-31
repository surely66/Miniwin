#include<ntvcommon.h>
#include<app.h>
#include <ngl_video.h>
#include<ngl_timer.h>
#include<ngl_log.h>

NGL_MODULE(NTVCOMMON)

namespace ntvplus{

NTVEditBox::NTVEditBox(int w,int h):EditBox(w,h){
    setBgColor(0xFF222222);
    setFgColor(0xFFFFFFFF);
}
void NTVEditBox::onDraw(GraphContext&canvas){
    setBgColor(hasFlag(Attr::ATTR_FOCUSED)?0xFF00FF00:0xFF222222);
    EditBox::onDraw(canvas);
}

NTVTitleBar::NTVTitleBar(int w,int h):View(w,h){
   setBgColor(0xFF222222);
   setFgColor(0xFFFFFFFF);
   logo=App::getInstance().loadImage("mainmenu_small_logo.png");
}

void NTVTitleBar::setTime(time_t tn){
   time_now=tn;
   invalidate(nullptr);
}

void NTVTitleBar::setTitle(const std::string&txt){
   title=txt;//App::getInstance().getString(txt);
   invalidate(nullptr);
}

void NTVTitleBar::onDraw(GraphContext&canvas){
    char buf[32];
    const char*weekday[]={"sunday","monday","tuesday","wednesday","thursday","friday","saturday"};
    RECT rect=getClientRect();
    View::onDraw(canvas);
    int xx=50;
    if(logo){
        RECT rcimg=rect;
        rcimg.x=50;
        rcimg.width=logo->get_width()*getHeight()/logo->get_height();
        canvas.draw_image(logo,&rcimg,nullptr,ST_CENTER_INSIDE);
        xx+=rcimg.width;
    }
    RefPtr<Gradient>pat=LinearGradient::create(xx,0,xx,getHeight());
    pat->add_color_stop_rgba(.0,1.,.0,.0,.2);
    pat->add_color_stop_rgba(.5,.0,1.,.0,1.);
    pat->add_color_stop_rgba(1.,.0,.0,1.,.2);
    canvas.set_source(pat);
    canvas.set_font_size(50);
    canvas.move_to(xx,0);
    canvas.line_to(xx,getHeight());
    canvas.rectangle(rect.x,rect.y,rect.width,rect.height);
    canvas.stroke();
    rect.x=xx+8;
    canvas.draw_text(rect,title,DT_LEFT|DT_VCENTER); 

    time_t tnow=time(NULL); 
    struct tm tmnow;
    gmtime_r(&tnow,&tmnow);
    TextExtents te1,te2;
    
    //printf("gmtime=%s ",asctime(gmtime(&tnow)));
    //printf("localtime=%s",asctime(localtime(&tnow)));
    
    sprintf(buf,"%02d:%02d:%02d",tmnow.tm_hour,tmnow.tm_min,tmnow.tm_sec);
    std::string stime=buf;
    sprintf(buf,"%02d/%02d/%d",tmnow.tm_mon+1,tmnow.tm_mday,1900+tmnow.tm_year);
    std::string sdate=buf;
    std::string sweek=weekday[tmnow.tm_wday];

    canvas.get_text_extents(stime,te2);
    rect.x=getWidth()-te2.width-50;
    rect.width=te2.width;
    canvas.draw_text(rect,stime);
    
    canvas.set_font_size(25); 
    canvas.get_text_extents(sdate,te1);
    rect.x=getWidth()-380;
    rect.width=te1.width+te1.height;
    sweek=App::getInstance().getString(sweek);
    canvas.draw_text(rect,sweek+"\n"+sdate,DT_CENTER|DT_VCENTER|DT_MULTILINE);
}


NTVSelector::NTVSelector(const std::string&txt,int w,int h):Selector(txt,w,h){
    setBgColor(0xFF000000);
    setFgColor(0xFFFFFFFF);
}

void NTVSelector::onDraw(GraphContext&canvas){
     Selector::onDraw(canvas);
     RefPtr<Gradient>pat=LinearGradient::create(0,0,getWidth(),getHeight());
     pat->add_color_stop_rgba(.0,.2,.2,.2,.2);
     pat->add_color_stop_rgba(.5,1.,1.,1.,1.);
     pat->add_color_stop_rgba(1.,.2,.2,.2,.2);
     canvas.set_source(pat);//color(0xFF888888);
     canvas.move_to(0,0);canvas.line_to(getWidth(),0);
     canvas.move_to(0,getHeight());canvas.move_to(getWidth(),getHeight());
     canvas.stroke();
}

NTVProgressBar::NTVProgressBar(int w,int h):ProgressBar(w,h){

}
void NTVProgressBar::onDraw(GraphContext&canvas){
     RECT r=getClientRect();
     RefPtr<Gradient>pat=LinearGradient::create(0,0,0,r.height);
     pat->add_color_stop_rgba(.0,.2,.2,.2,1.);
     pat->add_color_stop_rgba(.5,.0,.0,.0,1.);
     pat->add_color_stop_rgba(1.,.2,.2,.2,1.);
     canvas.set_source(pat);//color(0xFF888888);
     canvas.draw_rect(r);
     if(getWidth()>getHeight()){
         r.width=r.width*progress_/(max_-min_);
     }else{
         int h=r.height;
         r.height=r.height*progress_/(max_-min_);
         r.offset(0,h-r.height);
     }
     RefPtr<Gradient>pat1=LinearGradient::create(r.x,r.y,r.x,r.height);//height);
     pat1->add_color_stop_rgba(.0,.2,.2,.2,1.);
     pat1->add_color_stop_rgba(.5,1.,1.,1.,1.);
     pat1->add_color_stop_rgba(1.,.2,.2,.2,1.);
     canvas.set_source(pat1);
     canvas.draw_rect(r);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SettingPainter(AbsListView&lv,const ListView::ListItem&itm,int state,GraphContext&canvas){
    if(state){
        canvas.set_color(lv.hasFlag(View::Attr::ATTR_FOCUSED)?0xFF008000:0xFF004000);
    }else
        canvas.set_color(lv.getBgColor());
    RECT rect=itm.rect;
    rect.height-=1;
    canvas.draw_rect(rect);
    canvas.set_color(lv.getFgColor());
    rect.inflate(-20,0);
    canvas.draw_text(rect,itm.getText(),DT_LEFT|DT_VCENTER);
    rect.inflate(20,0);
    RefPtr<Gradient>pat=LinearGradient::create(rect.x,rect.y,rect.x+rect.width,rect.y+rect.height);
    pat->add_color_stop_rgba(.0,.2,.2,.2,.2);
    pat->add_color_stop_rgba(.5,1.,1.,1.,1.);
    pat->add_color_stop_rgba(1.,.2,.2,.2,.2);
    canvas.set_source(pat);//color(0xFF888888);

    rect.set(rect.x,rect.y+rect.height,rect.width,1);
    canvas.draw_rect(rect);
}

static void ChannelPainterInner(AbsListView&lv,const ListView::ListItem&itm,int state,GraphContext&canvas,bool hasid){
    ChannelItem&ch=(ChannelItem&)itm;
    RECT r=itm.rect;
    r.inflate(0,-1);
    if(state)
         canvas.set_color(0xFF008000);
    else{
         if(lv.getBgPattern())
            canvas.set_source(lv.getBgPattern());
         else canvas.set_color(lv.getBgColor());
    }
    canvas.draw_rect(r);
    canvas.set_color(lv.getFgColor());
    
    r.width=hasid?80:20;
    r.x=20;
    if(hasid){
        canvas.draw_text(r,std::to_string(itm.getValue()),DT_RIGHT|DT_VCENTER);
        r.x=r.x+r.width+20;
    }
    r.width=itm.rect.width-r.x;
    
    canvas.draw_text(r,itm.getText(),DT_LEFT|DT_VCENTER);
    r=itm.rect;
    r.inflate(6,0);
    RefPtr<Gradient>pat=LinearGradient::create(r.x,r.y,r.x+r.width,r.y+r.height);
    pat->add_color_stop_rgba(.0,.2,.2,.2,.2);
    pat->add_color_stop_rgba(.5,1.,1.,1.,1.);
    pat->add_color_stop_rgba(1.,.2,.2,.2,.2);
    canvas.set_source(pat);//color(0xFF888888);

    canvas.move_to(r.x,r.y);canvas.line_to(r.x+r.width,r.y);
    canvas.move_to(r.x,r.y+r.height);canvas.line_to(r.x+r.width,r.y+r.height);
    canvas.stroke();

    r.set(r.width-r.height,r.y,r.height,r.height);
    if(ch.camode)canvas.draw_text(r,"$",DT_LEFT|DT_VCENTER);
}

void  ChannelPainterLCN(AbsListView&lv,const ListView::ListItem&itm,int state,GraphContext&canvas){
    ChannelPainterInner(lv,itm,state,canvas,true);
}
void  ChannelPainter(AbsListView&lv,const ListView::ListItem&itm,int state,GraphContext&canvas){
    ChannelPainterInner(lv,itm,state,canvas,false);
}

ToolBar*CreateNTVToolBar(int w,int h){
    ToolBar*toolbar=new ToolBar(w,h);
    RefPtr<LinearGradient>pat=LinearGradient::create(0,0,w,h);
    pat->add_color_stop_rgba(.0,.0,.0,.0,1.);
    pat->add_color_stop_rgba(.5,.6,.6,.6,1.);
    pat->add_color_stop_rgba(1.,.0,.0,.0,1.);
    toolbar->setBgPattern(pat);
    return toolbar;
}
const std::string GetTPString(const TRANSPONDER*tp){
    char buf[128];
    const char*polars[]={"H","V","L","R"};
    sprintf(buf,"%d/%s/%d",tp->u.s.frequency/1000,polars[tp->u.s.polar],tp->u.s.symbol_rate);
    return buf;
}

void ShowVolumeWindow(int timeout){
    ToastWindow::makeWindow(400,20,0,[&](Window&w,int){
           int vol=nglSndGetColume(0);
           ProgressBar*p=new ProgressBar(390,10);
           w.addChildView(p);
           p->setProgress(vol);
           w.setPos(400,600);
       },timeout);
}
void ShowAudioSelector(int estype,int timeout){//ST_AUDIO
    ToastWindow::makeWindow(400,100,0,[&](Window&w,int){
           SERVICELOCATOR svc;
           ELEMENTSTREAM es[16];
           char str[16];
           DtvGetCurrentService(&svc);
           int cnt=DtvGetServiceElements(&svc,estype,es);
           ListView*lst=new ListView(390,80);
           for(int i=0;i<cnt;i++){
               NGLOG_DEBUG("audio[%d] pid=%d type=%d lan=%s",i,es[i].pid,es[i].getType(),es[i].iso639lan);
               if(es[i].iso639lan[0])
                   lst->addItem(new ListView::ListItem((const char*)es[i].iso639lan));
           }
           w.addChildView(lst);
           w.setPos(400,600);
       },timeout);
}

}//namespace
