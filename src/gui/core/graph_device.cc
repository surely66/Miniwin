#include <graph_context.h>
#include <graph_device.h>
#include <cairo.h>
#include <ngl_types.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <ngl_log.h>
#include <cairo-ngl.h>
#include <vector>
#include <cairomm/context.h>
#include <cairomm/ngl_surface.h>
#include <cairomm/region.h>
#include <freetype/freetype.h>

using namespace std;
using namespace Cairo;

NGL_MODULE(GraphDevice)

std::vector<GraphContext*>GraphDevice::gSurfaces;
GraphDevice*GraphDevice::mInst=nullptr;

GraphDevice*GraphDevice::GraphDevice::getInstance(){
    if(nullptr==mInst){
        DWORD tid;
        nglGraphInit();
        mInst=new GraphDevice();
        nglCreateThread(&tid,0,0,GraphDevice::ComposeProc,mInst);
    }
    return mInst;
}

GraphDevice::GraphDevice(int format){
    mInst=this; 
    nglGetScreenSize((UINT*)&width,(UINT*)&height);
    FT_Init_FreeType(&ft_library);
    
    graphEvent=nglCreateEvent(0,0);
    nglCreateSurface(&primarySurface,width,height,0,1);
    NGLOG_DEBUG("primarySurface=%p size=%dx%d graphEvent=%x ",primarySurface,width,height,graphEvent);

    cairo_surface_t*surface=cairo_ngl_surface_create(primarySurface);
    primaryContext=new GraphContext(mInst,RefPtr<NGLSurface>(new NGLSurface(surface,true)));
    FT_Face ft_face;
    FT_New_Face(ft_library,"/usr/lib/fonts/droid_chn.ttf",0,&ft_face);
    //FT_New_Face(ft_library,"./msyh.ttc",0,&ft_face);
    NGLOG_DEBUG("ft_library=%p ft_face=%p family=%s",ft_library,ft_face,ft_face->family_name);
    RefPtr<FontFace>face=FtFontFace::create(ft_face,FT_LOAD_FORCE_AUTOHINT);
    fonts[ft_face->family_name]=face;
}

RefPtr<const FontFace>GraphDevice::getFont(const std::string&family){
    RefPtr<const FontFace>face;
    if(!family.empty())
        face=fonts[family];
    if(face==nullptr)
        face=fonts.begin()->second;
    return face;
}

void GraphDevice::getScreenSize(int &w,int&h){
    w=width;
    h=height;
}

int GraphDevice::getScreenWidth(){
    return width;
}

int GraphDevice::getScreenHeight(){
    return height;
}

void GraphDevice::flip(GraphContext*ctx){
    if(ctx)ctx->get_target()->flush();//cairo_surface_flush(ctx->surface);
    nglSetEvent(graphEvent);
}

GraphContext*GraphDevice::getPrimaryContext(){
    return primaryContext;
}

GraphContext*GraphDevice::createContext(int width,int height){
    unsigned char*data;
    UINT pitch;
    DWORD nglsurface;
    nglCreateSurface(&nglsurface,width,height,0,0);
    cairo_surface_t*surface=cairo_ngl_surface_create(nglsurface);
    GraphContext*graph_ctx=new GraphContext(this,RefPtr<NGLSurface>(new NGLSurface(surface,true)));
    NGLOG_VERBOSE("nglsurface=%p cairo_surface=%p size=%dx%d content=%x operatopr=%d",nglsurface,surface,width,height,cairo_surface_get_content(surface),
        graph_ctx->get_operator());     
    gSurfaces.push_back(graph_ctx);
    graph_ctx->dev=this;
    //graph_ctx->set_operator(OPERATOR_SOURCE);
    graph_ctx->set_font_face(getFont());
    graph_ctx->set_antialias(ANTIALIAS_GRAY);//ANTIALIAS_SUBPIXEL);
    return graph_ctx;
}

GraphContext*GraphDevice::createContext(const RECT&rect){
    GraphContext*ctx=createContext(rect.width,rect.height);
    ctx->screenPos.set(rect.x,rect.y);
    return ctx;
}

void GraphDevice::remove(GraphContext*ctx){
    for(auto itr=gSurfaces.begin();itr!=gSurfaces.end();itr++){
       if((*itr)==ctx){
            gSurfaces.erase(itr);
            break;
       }
    }
    flip(nullptr);
}

static void getSurfaceRegion(GraphContext*c,POINT &pt,RectangleInt*r){
    unsigned int w,h,f;
    DWORD nglsurface =cairo_ngl_surface_get_surface(c->get_target()->cobj());
    nglGetSurfaceInfo(nglsurface,&w,&h,(int*)&f);
    r->x=pt.x;
    r->y=pt.y;
    r->width=w;
    r->height=h; 
}

bool GraphDevice::CheckOverlapped(GraphContext*c,int idx){
    RectangleInt rect;
    getSurfaceRegion(c,c->screenPos,&rect);
    RefPtr<Cairo::Region> region=Cairo::Region::create(rect);
    for(int i=idx+1;i<gSurfaces.size();i++){
         getSurfaceRegion(gSurfaces[i],gSurfaces[i]->screenPos,&rect);
         region->subtract(rect);
         if(region->empty()){
             return true;
         }
    }
    return false;
}

void GraphDevice::ComposeProc(void*param){
    GraphDevice*dev=(GraphDevice*)param;
    int writed=0;
    NGLOG_DEBUG("Compose Thread started primarySurface=%p graphEvent=%p....",dev->primarySurface,dev->graphEvent);
    while(true){
        if(nglWaitEvent(dev->graphEvent,100)==NGL_OK) {
            int i=0;
            NGLRect rr={40,70,320,240}; 
            nglFillRect(dev->primarySurface,nullptr,0);
            for(auto s:gSurfaces){
                POINT pt=s->screenPos;
                NGLRect srec={pt.x,pt.y,0,0};
                DWORD nglsurface=cairo_ngl_surface_get_surface(s->get_target()->cobj());
                //nglFillRect(nglsurface,&rr,0x00FF0000);
                if(GraphDevice::CheckOverlapped(s,i++))continue;
                nglBlit(dev->primarySurface,nglsurface,nullptr,&srec);
            }
            nglFlip(dev->primarySurface);
        }
    }
}

