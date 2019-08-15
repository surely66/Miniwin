#ifndef __LAYOUT_H__
#define __LAYOUT_H__
#include <view.h>
namespace nglui{

class Layout {
 public:
  Layout();
  explicit Layout(const RECT& margin);

  virtual ~Layout();

  virtual void setMargins(int thickness);
  virtual void setMargins(const RECT& margin);

  virtual void onLayout(class View* view) = 0;

 private:
  RECT margin_;
};

}
#endif

