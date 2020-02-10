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
#include <cairomm/fontface.h>
#include <ft2build.h>
#include <freetype/freetype.h>

using namespace std;
using namespace Cairo;

NGL_MODULE(GraphDevice)

namespace nglui{

std::vector<GraphContext*>GraphDevice::gSurfaces;
GraphDevice*GraphDevice::mInst=nullptr;

GraphDevice*GraphDevice::GraphDevice::getInstance(){
    if(nullptr==mInst){
        DWORD tid;
        nglGraphInit();
        mInst=new GraphDevice();
    }
    return mInst;
}

GraphDevice::GraphDevice(int format){
    mInst=this; 
    nglGetScreenSize((UINT*)&width,(UINT*)&height);
    FT_Init_FreeType(&ft_library);
    
    nglCreateSurface(&primarySurface,width,height,0,1);
    NGLOG_DEBUG("primarySurface=%p size=%dx%d ",primarySurface,width,height);

    cairo_surface_t*surface=cairo_ngl_surface_create(primarySurface);
    primaryContext=new GraphContext(mInst,RefPtr<NGLSurface>(new NGLSurface(surface,true)));
    FT_Face ft_face;
#if defined(__amd64__)||defined(__x86_64__)||defined(_M_AMD64)
    FT_New_Face(ft_library,"/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",0,&ft_face);
#else
    FT_New_Face(ft_library,"/usr/lib/fonts/droid_chn.ttf",0,&ft_face);
#endif
    NGLOG_DEBUG_IF(ft_face,"ft_library=%p ft_face=%p family=%s",ft_library,ft_face,ft_face?ft_face->family_name:nullptr);
    RefPtr<FontFace>face=FtFontFace::create(ft_face,FT_LOAD_FORCE_AUTOHINT);
    fonts[ft_face->family_name]=face;
}

RefPtr<const FontFace>GraphDevice::getFont(const std::string&family){
    RefPtr<const FontFace>face;
    if(!family.empty()&&fonts.size())
        face=fonts[family];
    if(face==nullptr&&fonts.size())
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
    //if(ctx)ctx->get_target()->flush();//cairo_surface_flush(ctx->surface);
    compose_event++;
    NGLOG_VERBOSE("flip %p",ctx);
}

GraphContext*GraphDevice::getPrimaryContext(){
    return primaryContext;
}

GraphContext*GraphDevice::createContext(int width,int height){
    unsigned char*data;
    UINT pitch;
    HANDLE nglsurface;
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
    HANDLE nglsurface =cairo_ngl_surface_get_surface(c->get_target()->cobj());
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

void GraphDevice::ComposeSurfaces(){
    int writed=0;
    int i=0;
    NGLRect rr={40,70,320,240}; 
    nglFillRect(primarySurface,nullptr,0);
    for(auto s:gSurfaces){
        POINT pt=s->screenPos;
	NGLRect drect={pt.x,pt.y,0,0};
        HANDLE nglsurface=cairo_ngl_surface_get_surface(s->get_target()->cobj());
        if(GraphDevice::CheckOverlapped(s,i++))continue;
	NGLOG_VERBOSE("Blit %p to %d,%d",s,pt.x,pt.y);
        nglBlit(primarySurface,&drect,nglsurface,nullptr);
    }
    nglFlip(primarySurface);
    compose_event=0;
}
}//end namespace
