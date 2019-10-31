#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__
#include <widget.h>
#include <groupview.h>
namespace nglui{
class ToolBar:public Widget{
protected:
   typedef struct{
       RefPtr<ImageSurface> image;
       std::string text;
       int pos;
       int width;
   }BUTTON;
   std::vector<BUTTON>buttons;
   int index_;
typedef Widget INHERITED;
public:
   ToolBar(int width, int height);
   virtual void addButton(const std::string&txt,int x=-1,int width=-1);
   virtual void addButton(const std::string&img,const std::string&txt,int x=-1,int width=-1);
   virtual void clearButtons();
   int getButtonCount();
   virtual void setIndex(int idx);
   virtual int getIndex();
   virtual void onDraw(GraphContext&canvas)override;
   virtual bool onKeyRelease(KeyEvent&k)override;
};

}//namespace
#endif
