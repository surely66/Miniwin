#include <ngl_types.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <ngl_log.h>
#include <directfb.h>
#include <directfb_util.h>
#include <direct/messages.h>
#include <aui_input.h>
#include <aui_osd.h>
#define MAX_SURFACE_COUNT 16

NGL_MODULE(GRAPH)

static IDirectFB *directfb=NULL;
//AUI porting has some crash roblem,but work's fine  
//#define USE_DIRECTFB 1

static int created_surface=0;
static int destroyed_surface=0;
static aui_hdl g_hw_layer;
DWORD nglGraphInit()
{
#ifdef USE_DIRECTFB
    if(directfb!=NULL)return NGL_OK;
    DirectFBInit (NULL,NULL);
    DirectFBCreate (&directfb);
    directfb->SetCooperativeLevel (directfb, DFSCL_FULLSCREEN);
    NGLOG_DEBUG("directfb=%p",directfb);
#else
    static int inited=0;
    if(0==inited){
        aui_gfx_init(NULL, NULL);
        inited++;
        //aui_log_priority_set(AUI_MODULE_GFX,AUI_LOG_PRIO_DEBUG);
    } 
#endif
    return NGL_OK;
}

static void NGLRect2Aui(NGLRect*r,struct aui_osd_rect*ar){
    ar->uLeft=r->x;
    ar->uTop=r->y;
    ar->uWidth=r->w;
    ar->uHeight=r->h;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
#ifdef USE_DIRECTFB
    IDirectFBDisplayLayer *dispLayer;
    DFBDisplayLayerConfig dispCfg;
    nglGraphInit();
    directfb->GetDisplayLayer( directfb, DLID_PRIMARY, &dispLayer );
    dispLayer->GetConfiguration( dispLayer, &dispCfg );
    *width=dispCfg.width;
    *height=dispCfg.height;
#else
    *width=1280;
    *height=720;
#endif
    return NGL_OK;
}

DWORD nglLockSurface(DWORD surface,void**buffer,UINT*pitch){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Lock(surf,DSLF_READ | DSLF_WRITE,buffer,pitch);
    NGLOG_VERBOSE_IF(ret,"surface=%p buffer=%p pitch=%d",surf,buffer,*pitch);
#else
    aui_surface_info info;
    int ret=aui_gfx_surface_lock((aui_hdl)surface);
    aui_gfx_surface_info_get((aui_hdl)surface,&info);
    *buffer=info.p_surface_buf;
    *pitch=info.pitch;
    NGLOG_VERBOSE_IF(ret,"surface=%p buffer=%p width=%d pitch=%d",surface,buffer,info.width,*pitch); 
#endif
    return ret;
}

DWORD nglGetSurfaceInfo(DWORD surface,UINT*width,UINT*height,INT *format)
{
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    if(NULL==width||NULL==height||NULL==format)
       return NGL_INVALID_PARA;
    int ret=surf->GetSize(surf,width,height);
    *format=GPF_ARGB;
    return ret;
#else
    aui_surface_info info;
    int ret=aui_gfx_surface_info_get((aui_hdl)surface,&info);
    *width=info.width;//p_surface_buf;
    *height=info.height;
    *format=GPF_ARGB;
#endif
    NGLOG_VERBOSE_IF(ret,"surface=%x,size=%dx%d format=%d ret=%d",surface,*width,*height,*format,ret);
    return NGL_OK;
}

DWORD nglUnlockSurface(DWORD surface){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Unlock(surf);
#else
    int ret=aui_gfx_surface_unlock((aui_hdl)surface);
#endif
    NGLOG_VERBOSE_IF(ret,"surface=%p ret=%d",surface,ret);
    return ret;
}

DWORD nglSurfaceSetOpacity(DWORD surface,BYTE alpha){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    IDirectFBDisplayLayer *dispLayer;
    directfb->GetDisplayLayer( directfb, DLID_PRIMARY, &dispLayer );
    return dispLayer->SetOpacity(dispLayer,alpha);
#else
    return aui_gfx_layer_alpha_set(g_hw_layer,alpha);
#endif
}

DWORD nglFillRect(DWORD surface,const NGLRect*rec,UINT color){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    NGLRect r={0,0,0,0};
    if(NULL==rec)
        surf->GetSize(surf,&r.w,&r.h);
    else
        r=*rec;
    surf->SetColor(surf,(color>>24),(color>>16),(color>>8),color);
    surf->FillRectangle(surf,r.x,r.y,r.w,r.h);
#else
    aui_osd_rect rc={0,0,0,0};
    UINT w,h,f;
    nglGetSurfaceInfo(surface,&w,&h,&f);
    if(rec==NULL){
        rc.uWidth=w;rc.uHeight=h;
    }else NGLRect2Aui(rec,&rc);
    aui_gfx_surface_fill(surface,color,&rc);
    NGLOG_DEBUG_IF((rc.uLeft+rc.uWidth>w)||(rc.uTop+rc.uHeight>h),"Filrect range error");
#endif
    return NGL_OK;
}

DWORD nglFlip(DWORD surface){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Flip( surf, NULL, DSFLIP_ONSYNC);
#else
    aui_osd_rect rc={0,0,0,0};
    UINT w,h,f;
    aui_surface_info info;
    int ret=aui_gfx_surface_info_get((aui_hdl)surface,&info);
    rc.uWidth=info.width;rc.uHeight=info.height;

    if(info.is_hw_surface)
    ret=aui_gfx_surface_flush(surface,&rc); 
    NGLOG_VERBOSE_IF(ret,"flip %x=%d ishw=%d size=%dx%d ret=%d",surface,ret,info.is_hw_surface,rc.uWidth,rc.uHeight,ret);
#endif
    return ret;
}

DWORD nglCreateSurface(DWORD*surface,INT width,INT height,INT format,BOOL hwsurface)
{
#ifdef USE_DIRECTFB
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
#else
     int ret;
     aui_hdl surf_handle;
     aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB8888;
     struct aui_osd_rect region_rect={0,0,1280,720};
     if(hwsurface){
         unsigned long layer_id = AUI_OSD_LAYER_GMA0;
         if (aui_find_dev_by_idx(AUI_MODULE_GFX, layer_id, &g_hw_layer)) 
	     ret = aui_gfx_layer_open(layer_id, (aui_hdl*)(&g_hw_layer));
         aui_gfx_layer_show_on_off(g_hw_layer, 1);
         ret=aui_gfx_hw_surface_create(g_hw_layer, pixel_format, &region_rect,&surf_handle, 1);
         aui_gfx_surface_fill(surf_handle,0, &region_rect);
     }else{
         ret=aui_gfx_sw_surface_create(pixel_format,width,height,&surf_handle);
     }
     region_rect.uWidth=width; region_rect.uHeight=height;
     aui_gfx_surface_clip_rect_set(surf_handle,&region_rect,AUI_GE_CLIP_INSIDE);
     aui_gfx_surface_fill(surf_handle,(hwsurface?0x0:0xFF000000),&region_rect);
     *surface=(DWORD)surf_handle;
     NGLOG_VERBOSE_IF(ret,"surface=%x  ishw=%d ret=%d",surf_handle,hwsurface,ret);
     return NGL_OK;
#endif
}


DWORD nglBlit(DWORD dstsurface,DWORD srcsurface,const NGLRect*srcrect,const NGLRect* dstrect)
{
#ifdef USE_DIRECTFB
     IDirectFBSurface*dfbsrc=(IDirectFBSurface*)srcsurface;
     IDirectFBSurface*dfbdst=(IDirectFBSurface*)dstsurface;
     int ret=dfbdst->Blit(dfbdst,dfbsrc,srcrect,(dstrect?dstrect->x:0),(dstrect?dstrect->y:0));
     NGLOG_VERBOSE_IF(ret,"dstsurface=%p srcsurface=%p ret=%d",dstsurface,srcsurface,ret);
     return ret;
#else
     aui_blit_operation blit_op;
     aui_blit_rect blit_rect;
     
     memset(&blit_rect, 0, sizeof(aui_blit_rect));
     memset(&blit_op, 0, sizeof(aui_blit_operation));

     struct aui_osd_rect*dr=&blit_rect.dst_rect;
     struct aui_osd_rect*sr=&blit_rect.fg_rect;
     int sw,sh,dw,dh,f;
     nglGetSurfaceInfo(dstsurface,&dw,&dh,&f);
     nglGetSurfaceInfo(srcsurface,&sw,&sh,&f);

     sr->uWidth=sw;sr->uHeight=sh;
     sr->uLeft=srcrect?srcrect->x:0;
     sr->uTop=srcrect?srcrect->y:0;

     dr->uLeft=dstrect?dstrect->x:0;      
     dr->uTop=dstrect?dstrect->y:0;
     dr->uWidth=sw;dr->uHeight=sh;
    
     blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
     int outrange=(dr->uLeft+dr->uWidth>dw)||(dr->uTop+dr->uHeight>dh);
     int ret=aui_gfx_surface_blit(dstsurface,srcsurface,NULL,&blit_op,&blit_rect);
     NGLOG_VERBOSE_IF(ret||outrange,"blit=%d dstsurface=%x  srcsurface=%x dst=%d,%d-%d,%d src=%d,%d,%d,%d outrange=%d",ret,dstsurface,srcsurface,
                      dr->uLeft,dr->uTop,dr->uWidth,dr->uHeight,sr->uLeft,sr->uTop,sr->uWidth,sr->uHeight,outrange);
#endif
     return ret;
}

DWORD nglDestroySurface(DWORD surface)
{
#ifdef USE_DIRECTFB
     destroyed_surface++;
     NGLOG_VERBOSE("surface=%p  created=%d destroyed=%d",surface,created_surface,destroyed_surface);
     IDirectFBSurface*dfbsurface=(IDirectFBSurface*)surface;
     return dfbsurface->Release(dfbsurface);
#else
     aui_gfx_surface_delete((aui_hdl)surface);
     return NGL_OK;     
#endif
}
