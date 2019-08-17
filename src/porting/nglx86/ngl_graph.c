#include <ngl_types.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <ngl_log.h>
#include <directfb.h>
#include <directfb_util.h>
#include <direct/messages.h>


NGL_MODULE(GRAPH)

static IDirectFB *directfb=NULL;
//AUI porting has some crash roblem,but work's fine  
#define USE_DIRECTFB 1

static int created_surface=0;
static int destroyed_surface=0;
DWORD nglGraphInit()
{
    if(directfb!=NULL)return NGL_OK;
    DirectFBInit (NULL,NULL);
    DirectFBCreate (&directfb);
    directfb->SetCooperativeLevel (directfb, DFSCL_FULLSCREEN);
    NGLOG_DEBUG("directfb=%p",directfb);
    return NGL_OK;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
    IDirectFBDisplayLayer *dispLayer;
    DFBDisplayLayerConfig dispCfg;
    nglGraphInit();
    directfb->GetDisplayLayer( directfb, DLID_PRIMARY, &dispLayer );
    dispLayer->GetConfiguration( dispLayer, &dispCfg );
    *width=dispCfg.width;
    *height=dispCfg.height;
    return NGL_OK;
}

DWORD nglLockSurface(DWORD surface,void**buffer,UINT*pitch){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Lock(surf,DSLF_READ | DSLF_WRITE,buffer,pitch);
    NGLOG_VERBOSE_IF(ret,"surface=%p buffer=%p pitch=%d",surf,buffer,*pitch);
    return ret;
}

DWORD nglGetSurfaceInfo(DWORD surface,UINT*width,UINT*height,INT *format)
{
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    if(NULL==width||NULL==height||NULL==format)
       return NGL_INVALID_PARA;
    int ret=surf->GetSize(surf,width,height);
    *format=GPF_ARGB;
    return ret;
    NGLOG_VERBOSE_IF(ret,"surface=%x,size=%dx%d format=%d ret=%d",surface,*width,*height,*format,ret);
    return NGL_OK;
}

DWORD nglUnlockSurface(DWORD surface){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Unlock(surf);
    NGLOG_VERBOSE_IF(ret,"surface=%p ret=%d",surface,ret);
    return ret;
}

DWORD nglSurfaceSetOpacity(DWORD surface,BYTE alpha){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    IDirectFBDisplayLayer *dispLayer;
    directfb->GetDisplayLayer( directfb, DLID_PRIMARY, &dispLayer );
    return dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD nglFillRect(DWORD surface,const NGLRect*rec,UINT color){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    NGLRect r={0,0,0,0};
    if(NULL==rec)
        surf->GetSize(surf,&r.w,&r.h);
    else
        r=*rec;
    surf->SetColor(surf,(color>>24),(color>>16),(color>>8),color);
    surf->FillRectangle(surf,r.x,r.y,r.w,r.h);
    return NGL_OK;
}

DWORD nglFlip(DWORD surface){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Flip( surf, NULL, DSFLIP_ONSYNC);
    return ret;
}

DWORD nglCreateSurface(DWORD*surface,INT width,INT height,INT format,BOOL hwsurface)
{
     int i,ret;
     DFBSurfaceDescription   desc;
     IDirectFBSurface*dfbsurface;
     DFBSurfaceID surface_id;
     void*data;
     int pitch;
     desc.flags  = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS| DSDESC_WIDTH | DSDESC_HEIGHT);
     if(hwsurface){
         desc.flags = DSDESC_CAPS;
         desc.caps=(DFBSurfaceCapabilities)(DSCAPS_PRIMARY |DSCAPS_DOUBLE|DSCAPS_FLIPPING|DSDESC_PIXELFORMAT);
     }else
         desc.caps   = DSDESC_WIDTH | DSDESC_HEIGHT|DSDESC_PIXELFORMAT; //DSCAPS_SHARED;// | DSCAPS_TRIPLE DSDESC_PIXELFORMAT;
     desc.width=width;
     desc.height=height;
     desc.pixelformat=DSPF_ARGB;
     ret=directfb->CreateSurface( directfb, &desc,&dfbsurface);
     if(!hwsurface)dfbsurface->MakeClient(dfbsurface);
     NGLOG_VERBOSE_IF(ret,"surface=%x  ishw=%d",dfbsurface,hwsurface);
     created_surface++;
     dfbsurface->Clear(dfbsurface,0,0,0,(hwsurface?00:0xFF));
     
     dfbsurface->SetBlittingFlags(dfbsurface,DSBLIT_SRC_COLORKEY);//DSBLIT_BLEND_COLORALPHA);//DSBLIT_BLEND_ALPHACHANNEL);//DSBLIT_NOFX);
     *surface=(DWORD)dfbsurface;
     return NGL_OK;
}


DWORD nglBlit(DWORD dstsurface,DWORD srcsurface,const NGLRect*srcrect,const NGLRect* dstrect)
{
     IDirectFBSurface*dfbsrc=(IDirectFBSurface*)srcsurface;
     IDirectFBSurface*dfbdst=(IDirectFBSurface*)dstsurface;
     int ret=dfbdst->Blit(dfbdst,dfbsrc,srcrect,(dstrect?dstrect->x:0),(dstrect?dstrect->y:0));
     NGLOG_VERBOSE_IF(ret,"dstsurface=%p srcsurface=%p ret=%d",dstsurface,srcsurface,ret);
     return ret;
}

DWORD nglDestroySurface(DWORD surface)
{
     destroyed_surface++;
     NGLOG_VERBOSE("surface=%p  created=%d destroyed=%d",surface,created_surface,destroyed_surface);
     IDirectFBSurface*dfbsurface=(IDirectFBSurface*)surface;
     return dfbsurface->Release(dfbsurface);
}
