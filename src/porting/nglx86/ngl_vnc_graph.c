#include <ngl_types.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <ngl_log.h>
#include <stdio.h>
#include <stdlib.h> /* getenv(), etc. */
#include <unistd.h> 
#include <rfb/rfb.h>
#include <rfb/keysym.h>

NGL_MODULE(GRAPH)

typedef struct{
   unsigned int width;
   unsigned int height;
   unsigned int pitch;
   void*data;
}NGLSURFACE;

static NGLSURFACE*hwsurface;
static rfbScreenInfoPtr rfbScreen;

typedef struct ClientData {
  rfbBool oldButton;
  int oldx,oldy;
} ClientData;

static void clientgone(rfbClientPtr cl){
  free(cl->clientData);
  cl->clientData = NULL;
}
static enum rfbNewClientAction new_vnc_client(rfbClientPtr cl)
{
  cl->clientData = (void*)calloc(sizeof(ClientData),1);
  cl->clientGoneHook = clientgone;
  NGLOG_DEBUG("New Client ");
  return RFB_CLIENT_ACCEPT;
}

DWORD nglGraphInit(){
    int x=0,y=0;
    DWORD tid;
    int width=1280,height=720;
    rfbScreen = rfbGetScreen(NULL,NULL/*&argc,argv*/,width,height,8,3,4);
    //atexit(graph_done);
    rfbScreen->autoPort=TRUE;
    rfbScreen->port=8315; 
    rfbScreen->desktopName = "LibVNCServer Example";
    rfbScreen->frameBuffer = (char*)malloc(width*height*4);
    rfbScreen->alwaysShared = TRUE;
    rfbScreen->ptrAddEvent = NULL;//doptr;
    rfbScreen->kbdAddEvent = NULL;//dokey;
    rfbScreen->newClientHook = new_vnc_client;
    rfbScreen->httpDir = "../webclients";
    rfbScreen->httpEnableProxyConnect = TRUE;
    rfbInitServer(rfbScreen); 
    rfbRunEventLoop(rfbScreen,-1,TRUE);//non block 
    NGLOG_DEBUG("VNC Server Inited rfbScreen=%p port=%d framebuffer=%p",rfbScreen,rfbScreen->port,rfbScreen->frameBuffer); 
    return NGL_OK;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
    *width = rfbScreen->width;
    *height = rfbScreen->height;
    NGLOG_DEBUG("size=%dx%d",*width,*height);
    return NGL_OK;
}

DWORD nglLockSurface(DWORD surface,void**buffer,UINT*pitch){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    *buffer=ngs->data?ngs->data:rfbScreen->frameBuffer;
    *pitch=ngs->pitch;
    //NGLOG_DEBUG("%p buffer=%p pitch=%d",ngs,*buffer,*pitch);
    return 0;
}

DWORD nglGetSurfaceInfo(DWORD surface,UINT*width,UINT*height,INT *format)
{
    //*height= gdk_pixbuf_get_height(pb);
    *format=GPF_ARGB;
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    *width=ngs->width;
    *height=ngs->height;
    return NGL_OK;
}

DWORD nglUnlockSurface(DWORD surface){
    return 0;
}

DWORD nglSurfaceSetOpacity(DWORD surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD nglFillRect(DWORD surface,const NGLRect*rec,UINT color){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    NGLOG_DEBUG("FillRect %p color=0x%x",ngs,color);
    return NGL_OK;
}

DWORD nglFlip(DWORD surface){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    //NGLOG_DEBUG("flip %p",ngs);
    return 0;
}

DWORD nglCreateSurface(DWORD*surface,INT width,INT height,INT format,BOOL ishwsurface)
{//XShmCreateImage XShmCreatePixmap
     NGLSURFACE*nglsurface=(NGLSURFACE*)malloc(sizeof(NGLSURFACE));
     if(ishwsurface){
         nglsurface->data=NULL;
         hwsurface=nglsurface;
     }else{
         nglsurface->data=malloc(width*height);
     }
     nglsurface->width=width;
     nglsurface->height=height;
     nglsurface->pitch=width*4;
     *surface=(DWORD)nglsurface;
     NGLOG_DEBUG("surface=%p/%x framebuffer=%p size=%dx%d hw=%d",nglsurface,*surface,nglsurface->data,width,height,ishwsurface);
     return NGL_OK;
}


DWORD nglBlit(DWORD dstsurface,DWORD srcsurface,const NGLRect*srcrect,const NGLRect* dstrect)
{
     unsigned int x,y,sx,sy,sw,sh,dx,dy;
     NGLSURFACE*ndst=(NGLSURFACE*)dstsurface;
     NGLSURFACE*nsrc=(NGLSURFACE*)srcsurface;
     UINT *pbs=(UINT*)nsrc->data;
     UINT *pbd=(UINT*)(ndst->data?ndst->data:rfbScreen->frameBuffer);
     sx=srcrect?srcrect->x:0;    sy=srcrect?srcrect->y:0;
     sw=nsrc->width;      sh=nsrc->height;
     dx=dstrect?dstrect->x:0;    dy=dstrect?dstrect->x:0;

     //NGLOG_DEBUG("Blit from %p->%p %d,%d-%d,%d to %p %d,%d buffer=%p->%p",nsrc,ndst,sx,sy,sw,sh,ndst,dx,dy,pbs,pbd);
     pbs+=sy*(nsrc->pitch>>2)+sx;
     pbd+=dy*(ndst->pitch>>2)+dx;
     //NGLOG_DEBUG("buffer %p->%p pitch=%d/%d",pbs,pbd,nsrc->pitch,ndst->pitch);
     for(y=0;y<sh;y++){
         //memcpy(pbd,pbs,(sw<<2));
         //for(x=0;x<1;x++)pbd[x]=pbs[x];
         //pbs+=(nsrc->pitch>>2);
         //pbd+=(ndst->pitch>>2);
     }
     return 0;
}

DWORD nglDestroySurface(DWORD surface)
{
     NGLSURFACE*ngs=(NGLSURFACE*)surface;
     if(ngs->data){
        free(ngs->data);
     }
     free(ngs);
     return 0;
}
