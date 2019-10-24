#ifndef __NTV_COMMON_H__
#define __NTV_COMMON_H__
#include<ngl_types.h>
#include<dvbepg.h>
#include<windows.h>
#define KEY_RED    NGL_KEY_F0
#define KEY_GREEN  NGL_KEY_F1
#define KEY_YELLOW NGL_KEY_F2
#define KEY_BLUE   NGL_KEY_F3
namespace ntvplus{

class NTVTitleBar:public View{
protected:
   unsigned long time_now;
   std::string title;
   RefPtr<ImageSurface>logo;
public:
   NTVTitleBar(int w,int h);
   void setTime(time_t n);
   void setTitle(const std::string&txt);
   virtual void onDraw(GraphContext&canvas)override;
};

class NTVEditBox:public EditBox{
public:
    NTVEditBox(int w,int h);
    virtual void onDraw(GraphContext&canvas);
};


class NTVToolBar:public ToolBar{
public:
   NTVToolBar(int w,int h);
   void onDraw(GraphContext&canvas);
};

class NTVSelector :public Selector{
public:
   NTVSelector(const std::string&txt,int w,int h);
   virtual void onDraw(GraphContext&canvas)override;
};

class NTVProgressBar:public ProgressBar{
public:
   NTVProgressBar(int w,int h);
   virtual void onDraw(GraphContext&canvas)override;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define CHANNEL_LIST_ITEM_HEIGHT 40
class ChannelItem:public ListView::ListItem{
public:
    SERVICELOCATOR svc;
    BOOL camode;
    BOOL isHD;
public :
    ChannelItem(const std::string&txt,const SERVICELOCATOR*loc,bool camd=false):ListView::ListItem(txt){
        svc=*loc;camode=camd;
    }
    virtual void onGetSize(AbsListView&lv,int* w,int* h)override{
        if(h)*h=CHANNEL_LIST_ITEM_HEIGHT;
    }
};

class SatelliteItem:public AbsListView::ListItem{
public :
   int id;
   SATELLITE satellite;
   SatelliteItem(const SATELLITE&sat,int id_)
         :AbsListView::ListItem(sat.name){
        satellite=sat;       id=id_;
   }
};

class TransponderItem:public AbsListView::ListItem{
public:
   TRANSPONDER tp;
   TransponderItem(const TRANSPONDER&t);
};

void ChannelPainter(AbsListView&lv,const ListView::ListItem&itm,int state,GraphContext&canvas);
void ChannelPainterLCN(AbsListView&lv,const ListView::ListItem&itm,int state,GraphContext&canvas);

void SettingPainter(AbsListView&lv,const ListView::ListItem&itm,int state,GraphContext&canvas);
const std::string GetTPString(const TRANSPONDER*tp);
void ShowVolumeWindow(int timeout);
void ShowAudioSelector(int estype,int timeout);

}//namespace

#endif
