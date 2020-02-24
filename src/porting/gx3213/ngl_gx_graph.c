#include <ngl_types.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <ngl_log.h>
#include <gxcore.h>
#include <av/avapi.h>
NGL_MODULE(GRAPH)

static int device_handle=0;
static int vpu_handle=0;

typedef struct{
   UINT width;
   UINT height;
   UINT pitch;
   int format;
   int ishw;
   void*buffer;
   void*hw_surface;
}NGLSURFACE;

DWORD nglGraphInit(){
    GxAvRect vp;
    vp.x=vp.y=0;
    vp.width=1280;
    vp.height=720;
    if(0==device_handle){
        device_handle = GxAVCreateDevice(0); 
        vpu_handle = GxAVOpenModule(device_handle, GXAV_MOD_VPU, 0);
        GxAvdev_SetLayerViewport(device_handle, vpu_handle, GX_LAYER_OSD,&vp);
    }
    return NGL_OK;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
    GxAvRect vp;
    GxAvdev_GetLayerViewport(device_handle, vpu_handle, GX_LAYER_OSD,&vp);
    *width=vp.width;//1280;//dispCfg.width;
    *height=vp.height;//720;//dispCfg.height;
    return NGL_OK;
}

DWORD nglLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    NGLSURFACE*fb=(NGLSURFACE*)surface;
    *buffer=fb->buffer;
    *pitch=fb->pitch;
    return 0;
}

DWORD nglGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format)
{
    NGLSURFACE*fb=(NGLSURFACE*)surface;
    *width =fb->width;
    *height=fb->height;
    *format=fb->format;
    return NGL_OK;
}

DWORD nglUnlockSurface(HANDLE surface){
    return 0;
}

DWORD nglSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    GxVpuProperty_Alpha Alpha;
    NGLSURFACE*fb=(NGLSURFACE*)surface;
    Alpha.surface = fb->hw_surface;
    Alpha.alpha.type = GX_ALPHA_PIXEL;
    Alpha.alpha.value =alpha;
    GxAVSetProperty(device_handle, vpu_handle, GxVpuPropertyID_Alpha, &Alpha, sizeof(GxVpuProperty_Alpha));
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD nglFillRect(HANDLE surface,const NGLRect*rect,UINT color){
    NGLSURFACE*fb=(NGLSURFACE*)surface;
    GxVpuProperty_FillRect FillRect;
    int ret = 0;
    GxVpuProperty_BeginUpdate begin = {0};
    GxVpuProperty_EndUpdate end = {0};

    memset(&FillRect, 0, sizeof(GxVpuProperty_FillRect));
    FillRect.color.r = (color >> 16) & 0xFF;
    FillRect.color.g = (color >> 8) & 0xFF;
    FillRect.color.b = color & 0xFF;
    FillRect.color.a = (color >> 24) & 0xFF;
 
    FillRect.rect.x = rect->x;
    FillRect.rect.y = rect->y;
    FillRect.rect.width = rect->w;
    FillRect.rect.height = rect->h;
    FillRect.is_ksurface = 1;
    FillRect.surface = fb->hw_surface;

    begin.max_job_num =1;
    ret = GxAVSetProperty(device_handle,vpu_handle,
                        GxVpuPropertyID_BeginUpdate,
                        &begin, sizeof(GxVpuProperty_BeginUpdate));
    ret = GxAVSetProperty(device_handle,vpu_handle,
                        GxVpuPropertyID_FillRect,
                        &FillRect, sizeof(GxVpuProperty_FillRect));

    NGLOG_DEBUG_IF(ret,"[GUI]Filled Rectangle failed!\n");

    ret = GxAVSetProperty(device_handle,vpu_handle, GxVpuPropertyID_EndUpdate,
                                &end, sizeof(GxVpuProperty_EndUpdate));
    return NGL_OK;
}

DWORD nglFlip(HANDLE surface){
    NGLSURFACE*fb=(NGLSURFACE*)surface;
    GxVpuProperty_FlipSurface FlipSurface = {0};
        //return;
    FlipSurface.layer = GX_LAYER_OSD;
    FlipSurface.surface =fb->hw_surface;
    GxAVSetProperty(device_handle,vpu_handle,GxVpuPropertyID_FlipSurface,
                      &FlipSurface, sizeof(GxVpuProperty_FlipSurface));

    return 0;
}

DWORD nglCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface)
{
     int ret;
     NGLSURFACE*fb;
     GxVpuProperty_CreateSurface CreateSurface = {0};
     GxHwMallocObj ap;

     if(format==GX_COLOR_FMT_ARGB8888)
         ap.size=width*height*4;
     else
         return NGL_ERROR;

     fb=malloc(sizeof(NGLSURFACE));
     GxCore_HwMalloc(&ap,MALLOC_NO_CACHE);
     memset(&CreateSurface, 0, sizeof(CreateSurface));
     CreateSurface.format = format;
     CreateSurface.width  = width;
     CreateSurface.height = height;
     CreateSurface.mode   = GX_SURFACE_MODE_IMAGE;
     CreateSurface.buffer = ap.kernel_p;

     ret = GxAVGetProperty(device_handle, vpu_handle, GxVpuPropertyID_CreateSurface,
                        &CreateSurface,  sizeof(GxVpuProperty_CreateSurface));
     fb->width=width;
     fb->height=height;
     fb->ishw=hwsurface;
     fb->buffer=ap.usr_p;
     fb->format=GPF_ARGB; 
     fb->hw_surface=CreateSurface.surface;
     if(hwsurface)
        GxAvdev_SetLayerMainSurface(device_handle, vpu_handle, GX_LAYER_OSD,fb->hw_surface);
     NGLOG_DEBUG("surface=%x ",surface);

     return NGL_OK;
}


DWORD nglBlit(HANDLE dstsurface,const NGLRect*dstrect,HANDLE srcsurface,const NGLRect*srcrect)
{
     int ret;
     NGLSURFACE*dfb=(NGLSURFACE*)dstsurface;
     NGLSURFACE*sfb=(NGLSURFACE*)srcsurface;

     GxVpuProperty_Blit Blit;
     GxVpuProperty_BeginUpdate Begin = {0};
     GxVpuProperty_EndUpdate End = {0};

     ret = GxAVSetProperty(device_handle,vpu_handle,GxVpuPropertyID_BeginUpdate, &Begin,
                                sizeof(GxVpuProperty_BeginUpdate));
     memset(&Blit, 0, sizeof(GxVpuProperty_Blit));

     Blit.srca.dst_format  = GX_COLOR_FMT_ARGB8888;//get_color_format(0, gui.config.bpp);
     Blit.srca.surface     = sfb->hw_surface;
     Blit.srca.is_ksurface = 1;
     Blit.srca.alpha       = 1;   //??
     Blit.srca.rect.x      = srcrect?srcrect->x:0;
     Blit.srca.rect.y      = srcrect?srcrect->y:0;
     Blit.srca.rect.width  = srcrect?srcrect->w:sfb->width;
     Blit.srca.rect.height = srcrect?srcrect->h:sfb->height;

     Blit.srcb.surface     = dfb->hw_surface;
     Blit.srcb.dst_format  = GX_COLOR_FMT_ARGB8888;//get_color_format(0, gui.config.bpp);
     Blit.srcb.is_ksurface = 1;
     Blit.srcb.rect        = Blit.srca.rect;
     Blit.srcb.rect.x      = dstrect?dstrect->x:0;
     Blit.srcb.rect.y      = dstrect?dstrect->y:0;

     Blit.dst = Blit.srcb;
     Blit.mode = GX_ALU_ROP_COPY;
     Blit.colorkey_info.mode = GX_BLIT_COLORKEY_BASIC_MODE;
     Blit.dst.alpha = 0xff;  // no effect.
     Blit.colorkey_info.src_colorkey_en = 0;

        //Blit.colorkey_info.dst_colorkey_en = 1; //no effect
        //Blit.colorkey_info.dst_colorkey = 0x00ff00;//no effect

     Blit.colorkey_info.src_colorkey_en = 1;
     Blit.mode = GX_ALU_MIX_TRUE;

     Blit.srca.modulator.premultiply_en = 1;
     Blit.srca.modulator.step1_en = 1;
     Blit.srca.modulator.step1_mode = MODULAT_CHANNEL_ALPHA_WITH_REGCOLOR_ALPHA;
     Blit.srca.modulator.reg_color = 0xFF;

     Blit.srcb.modulator.premultiply_en = 1;
     Blit.srcb.modulator.step1_en = 1;
     Blit.srcb.modulator.step1_mode = MODULAT_CHANNEL_ALPHA_WITH_REGCOLOR_ALPHA;
     Blit.srcb.modulator.reg_color = 0xFF;

     Blit.dst.modulator.premultiply_en = 1;
     Blit.dst.modulator.step1_en = 1;
     Blit.dst.modulator.step1_mode = MODULAT_CHANNEL_ALPHA_WITH_REGCOLOR_ALPHA;
     Blit.dst.modulator.reg_color = 0xFF;

     ret = GxAVSetProperty(device_handle,vpu_handle, GxVpuPropertyID_Blit, &Blit,
                        sizeof(GxVpuProperty_Blit));

     ret |= GxAVSetProperty(device_handle,vpu_handle,GxVpuPropertyID_EndUpdate,&End,
                         sizeof(GxVpuProperty_EndUpdate));

     return 0;
}

DWORD nglDestroySurface(HANDLE surface)
{
     int ret;
     NGLSURFACE*fb=(NGLSURFACE*)surface;
     GxVpuProperty_DestroySurface DesSurface = { 0 };
     memset(&DesSurface, 0, sizeof(DesSurface));
     DesSurface.surface = fb->hw_surface;

     ret = GxAVSetProperty(device_handle,vpu_handle,
                                GxVpuPropertyID_DestroySurface,
                                &DesSurface,
                                sizeof(GxVpuProperty_DestroySurface));
     free(fb);
     return 0;
}
