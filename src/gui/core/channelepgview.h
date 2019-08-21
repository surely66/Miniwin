#ifndef __CHANNEL_EPG_VIEW_H__
#define __CHANNEL_EPG_VIEW_H__
#include <listview.h>
namespace nglui{

struct TVEvent{
   UINT id;
   std::string name;
   ULONG start;
   ULONG duration;
};

class ChannelBar:public ListView::ListItem{
public:
    std::vector<TVEvent>events;
public:
    ChannelBar(const std::string& txt);
    void addEvent(const TVEvent&evt);
    void addEvent(const std::string name,ULONG starttm,ULONG dur);
    void clearEvents();
    void onDraw(AbsListView&lv,GraphContext&canvas,const RECT&rect,bool isselected);
    void onGetSize(AbsListView&lv,int*w,int*h)override;
};

class ChannelEpgView:public ListView{
public:
    enum{
      RULER_BG,
      RULER_FG,
      CHANNEL_BG,
      CHANNEL_FG,
      CHANNEL_BAR,
      CHANNEL_FOCUSED,
      EVENT_POST,
      EVENT_NOW,
      EVENT_NEXT,
      EVENT_FEATURE,
      COLOR_LAST
    };
protected:
    ULONG starttime;
    float pixelMinute;
    int nameWidth;
    int timeRuleHeight;
    int colors[COLOR_LAST];
    void drawTimeRule(GraphContext&canvas);
public:
    ChannelEpgView(int w,int h);
    void setColor(int idx,UINT color);
    UINT getColor(int idx);
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

