#include <layout.h>

namespace nglui{

Layout::Layout() {
   margin_.set(0,0,0,0);
}

Layout::Layout(const RECT& margin) {
  margin_ = margin;
}

Layout::~Layout() {
}

void Layout::setMargins(int thickness) {
   margin_.set(thickness,thickness,thickness,thickness);
}

void Layout::setMargins(const RECT& margin) {
  margin_ = margin;
}

}//endof namespace
