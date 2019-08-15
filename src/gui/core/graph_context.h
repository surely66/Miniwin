#ifndef __GRAPH_CONTEXT_H__
#define __GRAPH_CONTEXT_H__
#include <ngl_types.h>
#include <ngl_graph.h>
#include <cairomm/context.h>
#include <graph_device.h>
using namespace Cairo;

enum SCALETYPE{
    ST_CENTER,
    ST_CENTER_CROP,
    ST_CENTER_INSIDE,
    ST_MATRIX,
    ST_FIT_CENTER,
    ST_FIT_START,
    ST_FIT_END,
    ST_FIT_XY,
};//SCALETYPE;
enum TEXTALIGNMENT{
    DT_LEFT   =0x00,
    DT_CENTER =0x01,
    DT_RIGHT  =0x02,
    DT_TOP    =0x00,
    DT_VCENTER=0x10,
    DT_BOTTOM =0x20,
    DT_MULTILINE=0x100
};
#define RGBA(r,g,b) (((a)<<24)|((r)<<16)|((g)<<8)|(b))
#define RGB(r,g,b,a) RGBA(r,g,b,0xFF)

class GraphContext:public Cairo::Context{
protected:
    class GraphDevice*dev;
    POINT screenPos;
    void*priv;
    friend class GraphDevice;
public:
    GraphContext(GraphDevice*_dev,const RefPtr<Surface>& target);
    GraphContext(GraphDevice*_dev,cairo_t* cobject, bool has_reference);
    GraphContext(GraphContext&ctx,int x,int y,int w,int h);
    ~GraphContext();
    void set_position(int x,int y);
    void draw_text(const RECT&rect,const std::string&text,int text_alignment=DT_LEFT|DT_VCENTER);
    void draw_text(int x,int y,const std::string& text);
    void get_text_size(const std::string&txt,int*w,int*h); 
    void draw_rect(int x,int y,int w,int h);
    void draw_rect(const RECT&rect);
    void draw_arraw(const RECT&r,int dir);//dir:0-up 1-down 2-left 3-right
    void set_color(BYTE r,BYTE g, BYTE b,BYTE a=255);
    void set_color(UINT color);
    void roundrect(int x,int y,int w,int h,int r=0);
    void rectangle(int x,int y,int w,int h);
    void rectangle(const RECT &r);
    void circle(int x,int y,int r);
    void draw_image(const RefPtr<ImageSurface>&img,int x,int y);
    void draw_image(const RefPtr<ImageSurface>&img,const RECT*dst,const RECT*src=nullptr,SCALETYPE st=ST_MATRIX);
    void draw_focus_rect(int x,int y,int w,int h);
    void flip();
    void dump2png(const char*fname);
};

#endif

