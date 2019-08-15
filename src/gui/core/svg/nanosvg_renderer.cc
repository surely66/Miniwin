#include <ngl_types.h>
#include <ngl_log.h>
#include<cairomm/context.h>
#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"
#include <cairo-svg.h>
NGL_MODULE(SVG_RENDER)
//reference https://github.com/pltxtra/libsvgandroid/blob/master/src_jni/libsvg-android/svg_android_render_helper.c
//
using namespace Cairo;
static void setPainter(RefPtr<Context>cr,NSVGpaint&paint){
    switch(paint.type){
    case NSVG_PAINT_NONE:break;
    case NSVG_PAINT_COLOR:{
             UINT c=paint.color;
             cr->set_source_rgb((c>>16)/255.,(c>>8)/255.,(c&0xFF)/255.);
         }break;
    case NSVG_PAINT_LINEAR_GRADIENT:{
             float*fp=paint.gradient->xform;
             RefPtr<Pattern>p=LinearGradient::create(fp[4],fp[5],fp[2]+fp[4],fp[3]+fp[5]);
             cr->set_source(p); NGLOG_DEBUG("LINEAR_GRADIENT");
         }break;
    case NSVG_PAINT_RADIAL_GRADIENT:{
             //RefPtr<Pattern>p=RadialGradient::create(
         }break;
    }
}
static void renderShape(RefPtr<Context>cr,NSVGshape*shape){
    NSVGpath*p=shape->paths;
    cr->move_to(p->pts[0],p->pts[1]);
    for(;p;p=p->next){
          cr->begin_new_path();
          cr->move_to(p->pts[0],p->pts[1]);
          for(int j=0;j<p->npts-1;j+=3){
             float*fp=&p->pts[j+j];
             cr->curve_to(fp[0],fp[1],fp[2],fp[3],fp[4],fp[5]);
          }
          if(p->closed){
              cr->line_to(p->pts[0],p->pts[1]);  
          }
          cr->close_path();
    }
}
static void Rasterize(RefPtr<ImageSurface>img,NSVGimage*image){
    RefPtr<Context>cr=Context::create(img);
    
    for (NSVGshape*shape = image->shapes; shape != NULL; shape = shape->next){
        if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;
        cr->set_line_join((LineJoin)shape->strokeLineJoin);
        cr->set_line_cap((LineCap)shape->strokeLineCap);
        renderShape(cr,shape);
        if(shape->fill.type != NSVG_PAINT_NONE){
            setPainter(cr,shape->fill);
            cr->fill_preserve();                
        }
        if(shape->stroke.type != NSVG_PAINT_NONE){
            setPainter(cr,shape->stroke);
            cr->stroke();
        }
    }
}
RefPtr<ImageSurface> nano_loadSVG(const char*buffer,size_t buffer_size){
#if 0
    char*bufp=(char*)malloc(buffer_size+1);
    memcpy(bufp,buffer,buffer_size);
    bufp[buffer_size]=0;
    NSVGimage *image=nsvgParse((char*)bufp,"px", 96.0f);
    NSVGshape*s=image->shapes;
    NGLOG_DEBUG("nanosvg image.size=%dx%d ",(int)image->width,(int)image->height);
    RefPtr<ImageSurface>img=ImageSurface::create(FORMAT_ARGB32,(int)image->width,(int)image->height);
    Rasterize(img,image);
    return img;
#else
#endif

}
