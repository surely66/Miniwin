#ifndef __LAYOUT_H__
#define __LAYOUT_H__
#include <view.h>
namespace nglui{
#define PADDING_LEFT   0
#define PADDING_RIGHT  1
#define PADDING_TOP    2
#define PADDING_BOTTOM 3
enum class GRAVITY:int{
   LEFT,
   RIGHT,
   TOP,
   BOTTOM
};
enum class MARGIN:int{
   LEFT,
   RIGHT,
   TOP,
   BOTTOM
};
class Layout {
public:
  Layout();

  virtual ~Layout();
  virtual void setGravity(GRAVITY g){gravity_=g;}
  virtual GRAVITY getGravity(){return gravity_;}
  virtual void setMarginLeft(int v){margins_[0]=v;};
  virtual void setMarginRight(int v){margins_[1]=v;};
  virtual void setMarginTop(int v){margins_[2]=v;};
  virtual void setMarginBottom(int v){margins_[3]=v;};
  virtual int getMarginLeft(){return margins_[0];}
  virtual int getMarginRight(){return margins_[1];}
  virtual int getMarginTop(){return margins_[2];}
  virtual int getMarginBottom(){return margins_[3];}
  virtual void setMargins(int v);
  
  virtual void setPaddingLeft(int v){paddings_[0]=v;}
  virtual void setPaddingRight(int v){paddings_[1]=v;}
  virtual void setPaddingTop(int v){paddings_[2]=v;}
  virtual void setPaddingBottom(int v){paddings_[3]=v;}
  virtual int getPaddingLeft(){return paddings_[0];}
  virtual int getPaddingRight(){return paddings_[1];}
  virtual int getPaddingTop(){return paddings_[2];}
  virtual int getPaddingBottom(){return paddings_[3];}
  virtual void setPaddings(int v);
  virtual void onLayout(class View* view) = 0;

 protected:
  int margins_[4];
  int paddings_[4];
  GRAVITY gravity_;
};

}
#endif

