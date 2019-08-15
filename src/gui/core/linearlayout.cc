#include "linearlayout.h"
#include <ngl_log.h>
NGL_MODULE(LinearLayout)
namespace nglui {

LinearLayout::LinearLayout()
  : Layout() {;
}

LinearLayout::LinearLayout(const RECT& margin)
  : Layout(margin) {
}

LinearLayout::~LinearLayout() {
}

void LinearLayout::onLayout(View* view) {
    int x = DELIMITER;
    int y = DELIMITER;
    int w = view->getWidth();
    int h = view->getHeight();
    int e_x = x + w;
    int e_y = y + h;//endof y of current layout line
    int dx = x;
    int dy = y;
    int max_height = 0;

    for (int i=0;i<view->getChildrenCount();i++){
        View*child=view->getChildView(i);
        // Get its prefer size
        SIZE size = child->getPreferSize();

        // Check the required size of the view, must be smaller/equal to,
        // and then skip it
        if(child->isVisible()==false)continue;
        if (size.width() > w || size.height() > h) {
            child->setBound(0, 0, 0, 0);
            NGLOG_ERROR("Size is too large, shrink to 0");
            continue;
        }

        // If not wide enough, move to next line
        if (dx + size.width() > e_x) {
            dx = x;
            dy += (max_height + DELIMITER);
            max_height=size.height();
        }

        // If not height enough, shrink it to 0 size, and then skip it
        if (dy + size.height() > e_y) {
            child->setBound(0, 0, 0, 0);
            NGLOG_DEBUG("%d,%d-%d,%d Out of Y(%d) lineheight=%d, shrink to 0",dx,dy,size.width(),size.height() ,e_y,max_height);
            continue;
        }

        // Maintain the maximum height of this line
        if (size.height() > max_height) {
            max_height = size.height();
        }

        // Set the view to a XY position
        child->setPos(dx, dy);

        NGLOG_VERBOSE("layout %p Child %p id=%d/%d pos(%d,%d-%d,%d)",this,child,child->getId(),i, dx,dy,size.x,size.y);
        // Move to next X position
        dx += (size.width() + DELIMITER);

        // If X has exceeded the boundary, then move to next line
        if (dx >= e_x) {
            dx = x;
            dy += (max_height + DELIMITER);
            max_height = 0;
        }
    }
}//  endof onLayout

}//endof namespace
