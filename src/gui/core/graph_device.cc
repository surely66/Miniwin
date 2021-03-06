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
#include <fontmanager.h>
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
        FontManager::getInstance().loadFonts("/usr/share/fonts");
        FontManager::getInstance().loadFonts("/usr/lib/fonts");
    }
    return mInst;
}

GraphDevice::GraphDevice(int format){
    mInst=this; 
    nglGetScreenSize((UINT*)&width,(UINT*)&height);
    FT_Init_FreeType(&ft_library);
    
    nglCreateSurface(&primarySurface,width,height,0,1);
    NGLOG_DEBUG("primarySurface=%p size=%dx%d ",primarySurface,width,height);

    primaryContext=new GraphContext(mInst,NGLSurface::create(primarySurface));
    std::vector<std::string>families;
    FontManager::getInstance().getFamilies(families);
    for(int i=0;i<families.size();i++){
        FT_Face ft_face;
        std::string path=FontManager::getInstance().getFontFile(families[i]);
        FT_New_Face(ft_library,path.c_str(),0,&ft_face);
        RefPtr<FontFace>face=FtFontFace::create(ft_face,FT_LOAD_FORCE_AUTOHINT);
        fonts[families[i]]=face;
    }
}

RefPtr<const FontFace>GraphDevice::getFont(const std::string&family){
    RefPtr<const FontFace>face;
    if(fonts.size()){
        face=fonts.begin()->second;//family.empty()?fonts.begin()->second:fonts[family];
    }
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
    HANDLE nglsurface;
    nglCreateSurface(&nglsurface,width,height,0,0);
    GraphContext*graph_ctx=new GraphContext(this,NGLSurface::create(nglsurface));
    NGLOG_VERBOSE("nglsurface=%p  size=%dx%d",nglsurface,width,height);     
    gSurfaces.push_back(graph_ctx);
    graph_ctx->dev=this;
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
