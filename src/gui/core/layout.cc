#include <layout.h>

namespace nglui{

Layout::Layout() {
   gravity_=GRAVITY::LEFT;
   setMargins(0);
   setPaddings(0);
}

Layout::~Layout() {
}

void Layout::setMargins(int v) {
    for(int i=0;i<4;i++)margins_[i]=v;
}
void Layout::setPaddings(int v){
    for(int i=0;i<4;i++)paddings_[i]=v;
}

}//endof namespace
