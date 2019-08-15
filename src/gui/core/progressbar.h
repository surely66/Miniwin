#ifndef __PROGRESS_BAR_H__
#define __PROGRESS_BAR_H__
#include <widget.h>

namespace nglui{

class ProgressBar:public Widget{
protected:
    int min_;
    int max_;
    int progress_;
public:
    ProgressBar(int width, int height);
    void setMin(int value);
    void setMax(int value);
    void setRange(int vmin,int vmax);
    void setProgress(int value);
    int getProgress();
    virtual void onDraw(GraphContext&canvas)override;
};

}
#endif
