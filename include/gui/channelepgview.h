#ifndef __CHANNEL_EPG_VIEW_H__
#define __CHANNEL_EPG_VIEW_H__
#include <listview.h>
namespace nglui{

struct TVEvent{
   std::string name;
   ULONG start;
   ULONG duration;
};

class ChannelBar:public ListView::ListItem{
public:
    enum{
      RULER_BK,
      RULER_LINE,
      CHANNEL_BK,
      CHANNEL_TXT,
      CHANNEL_BAR
    };
    std::vector<TVEvent>events;
public:
    ChannelBar(const std::string& txt):ListView::ListItem(txt){}
    void addEvent(const TVEvent&evt);
    void addEvent(const std::string name,ULONG starttm,ULONG dur);
    void clearEvents();
    void onDraw(AbsListView&lv,GraphContext&canvas,const RECT&rect,bool isselected);
    void onGetSize(AbsListView&lv,int*w,int*h)override;
};

class ChannelEpgView:public ListView{
protected:
    ULONG starttime;
    float pixelMinute;
    int nameWidth;
    int timeRuleHeight;
    void drawTimeRule(GraphContext&canvas);
public:
    ChannelEpgView(int w,int h);
    void setStartTime(ULONG t);
    void setPixelPerMinute(float p);
    void setChannelNameWidth(int w);
    int getChannelNameWidth();
    void setTimeRuleHeight(int h);
    int getTimeRuleHeight();
    ULONG getStartTime();
    float getPixelPerMinute();
    virtual void onDraw(GraphContext&canvas)override; 
    static void DefaultPainter(AbsListView&lv,const AbsListView::ListItem&itm,int state,GraphContext&canvas); 
public:
   typedef ListView INHERITED;
};

}//namespace
#endif

