#ifndef __PLACE_HOLDER_H__
#define __PLACE_HOLDER_H__
#include <view.h>
namespace nglui{

class PlaceHolder:public View{
public:
   PlaceHolder(int w,int h):View(w,h){
      setFlag(Attr::ATTR_TRANSPARENT);      
   }
};

}
#endif
