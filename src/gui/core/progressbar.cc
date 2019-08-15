#include <progressbar.h>
namespace nglui{

ProgressBar::ProgressBar(int width, int height):Widget(width,height){
    min_=0;
    max_=100;
}
void ProgressBar::setMin(int value){
    if(min_!=value){
        min_=value;
        invalidate(nullptr);
    }
}
void ProgressBar::setMax(int value){
    if(max_!=value){
        max_=value;
        invalidate(nullptr);
    }
}
void ProgressBar::setRange(int vmin,int vmax){
    if( (min_!=vmin)||(max_!=vmax)){
        min_=vmin;
        max_=vmax;
        invalidate(nullptr);
    }
}

void ProgressBar::setProgress(int value){
    if(progress_!=value){
        progress_=value;
        invalidate(nullptr);
    }
}

int ProgressBar::getProgress(){
    return progress_;
}

void ProgressBar::onDraw(GraphContext&canvas){
    RECT rect=getClientRect();
    canvas.set_color(getBgColor());
    canvas.draw_rect(rect);

    canvas.set_color(getFgColor());
    canvas.rectangle(rect);
    canvas.stroke();

    RECT r=rect;
    r.inflate(-1,-1);
    canvas.set_font_size(getFontSize());
    if(getWidth()>getHeight()){
         r.width=r.width*progress_/(max_-min_);
    }else{
        int h=r.height;
        r.height=r.height*progress_/(max_-min_);
        r.offset(0,h-r.height);
    } 
    canvas.set_source_rgb(.0,.0,1.);
    canvas.draw_rect(r);
}

}//end namespace
