#ifndef __NGLUI_LISTVIEW_H__
#define __NGLUI_LISTVIEW_H__
#include <abslistview.h>

namespace nglui{

class ListView:public AbsListView{
protected:
public:
   ListView(int w,int h);
   virtual void onDraw(GraphContext&canvas)override;
   virtual bool onKeyRelease(KeyEvent&k)override;
   virtual void setIndex(int idx)override;
typedef AbsListView INHERITED;
};

}//namespace
#endif
