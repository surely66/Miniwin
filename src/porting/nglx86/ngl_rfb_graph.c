#include <ngl_types.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <ngl_log.h>
#include <stdio.h>
#include <stdlib.h> /* getenv(), etc. */
#include <unistd.h> 
#include <sys/time.h>
#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include <rfb/rfbproto.h>
#include <ngl_ir.h>
NGL_MODULE(GRAPH)

typedef struct{
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    void*data;
    unsigned int ishw;
}NGLSURFACE;
typedef struct ClientData {
    rfbBool oldButton;
    int oldx,oldy;
} RFBClientData;

static NGLSURFACE*hwsurface;
static rfbScreenInfoPtr rfbScreen=NULL;

void SENDKEY(int k){
   NGLKEYINFO ki;
   ki.key_code=k;
   ki.repeat=1;
   ki.state=NGL_KEY_RELEASE;//NGL_KEY_PRESSED:NGL_KEY_RELEASE;
   nglIrSendKey(NULL,&ki,10);
}

static void onClientDone(rfbClientPtr cl){
   free(cl->clientData);
   cl->clientData = NULL;
}

static enum rfbNewClientAction onNewClient(rfbClientPtr cl){
    cl->clientData = (void*)calloc(sizeof(RFBClientData),1);
    cl->clientGoneHook = onClientDone;
    return RFB_CLIENT_ACCEPT;
}

static void onMousePtr(int buttonMask,int x,int y,rfbClientPtr cl){
   RFBClientData* cd=cl->clientData;
   if(x>=0 && y>=0 && x<rfbScreen->width && y<rfbScreen->height) {
      if(buttonMask) {
         int i,j,x1,x2,y1,y2;

         if(cd->oldButton==buttonMask) { /* draw a line */
            rfbDrawLine(cl->screen,x,y,cd->oldx,cd->oldy,0x00FF);
            x1=x; y1=y;
            if(x1>cd->oldx) x1++; else cd->oldx++;
            if(y1>cd->oldy) y1++; else cd->oldy++;
            rfbMarkRectAsModified(cl->screen,x,y,cd->oldx,cd->oldy);
         } else { /* draw a point (diameter depends on button) */
            int w=cl->screen->paddedWidthInBytes;
            x1=x-buttonMask; if(x1<0) x1=0;
            x2=x+buttonMask; if(x2>rfbScreen->width) x2=rfbScreen->width;
            y1=y-buttonMask; if(y1<0) y1=0;
            y2=y+buttonMask; if(y2>rfbScreen->height) y2=rfbScreen->height;

            for(i=x1*4/*BPP*/;i<x2*4;i++)
              for(j=y1;j<y2;j++)
                cl->screen->frameBuffer[j*w+i]=(char)0xff;
            rfbMarkRectAsModified(cl->screen,x1,y1,x2,y2);
         }
      } else
        cd->oldButton=0;
      cd->oldx=x; cd->oldy=y; cd->oldButton=buttonMask;
   }
   rfbDefaultPtrAddEvent(buttonMask,x,y,cl);
}
		 
static void onVNCClientKey(rfbBool down,rfbKeySym key,rfbClientPtr cl)
{
    if(down) {
	int x;
        NGLOG_DEBUG("rcv KEY %d",key);
        switch(key){
	case XK_F1:rfbCloseClient(cl);break;
        case XK_F2:SENDKEY(NGL_KEY_MENU);break;
        case XK_F5:
		 rfbMarkRectAsModified(cl->screen,0,0,rfbScreen->width,rfbScreen->height);break;
        case XK_F12: rfbShutdownServer(cl->screen,TRUE);break;
        case XK_F11:
        case XK_Page_Up:
        case XK_Up   : SENDKEY(NGL_KEY_UP)  ;break;
        case XK_Down : SENDKEY(NGL_KEY_DOWN);break;
        case XK_Left : SENDKEY(NGL_KEY_LEFT);break;
        case XK_Right: SENDKEY(NGL_KEY_RIGHT);break;
        case XK_BackSpace:SENDKEY(NGL_KEY_BACKSPACE);break;
        case XK_Escape:SENDKEY(NGL_KEY_ESCAPE);break;
        case XK_Menu : SENDKEY(NGL_KEY_MENU);break;
        case XK_space: SENDKEY(NGL_KEY_SPACE);break;
        case XK_Return:SENDKEY(NGL_KEY_ENTER);break;
        case XK_0     :SENDKEY(NGL_KEY_0);break;
        case XK_1     :SENDKEY(NGL_KEY_1);break;
        case XK_2     :SENDKEY(NGL_KEY_2);break;
        case XK_3     :SENDKEY(NGL_KEY_3);break;
        case XK_4     :SENDKEY(NGL_KEY_4);break;
        case XK_5     :SENDKEY(NGL_KEY_5);break;
        case XK_6     :SENDKEY(NGL_KEY_6);break;
        case XK_7     :SENDKEY(NGL_KEY_7);break;
        case XK_8     :SENDKEY(NGL_KEY_8);break;
        case XK_9     :SENDKEY(NGL_KEY_9);break;
        }
    }
}
static int  FileTransferPermitted(struct _rfbClientRec* cl){
    NGLOG_DEBUG("___");

    return TRUE;
}
DWORD nglGraphInit(){
    int x=0,y=0,*fb;
    HANDLE tid;
    int width=1280,height=720;
    if(rfbScreen)return NGL_OK;
    InitFileTransfer();    SetFtpRoot("/");
    rfbScreen = rfbGetScreen(NULL,NULL,width,height,8,3,4);
    rfbScreen->desktopName = "NGL-RFB";
    rfbScreen->frameBuffer = (char*)malloc(width*height*4);
    rfbScreen->alwaysShared = (1==1);
    rfbScreen->kbdAddEvent=onVNCClientKey;
    rfbScreen->ptrAddEvent=onMousePtr;
    rfbScreen->newClientHook=onNewClient;
    rfbScreen->permitFileTransfer=TRUE;
    rfbScreen->getFileTransferPermission=FileTransferPermitted;
    rfbRegisterTightVNCFileTransferExtension();
    rfbInitServer(rfbScreen);
    rfbRunEventLoop(rfbScreen,5,TRUE);//non block 
    NGLOG_DEBUG("VNC Server Inited rfbScreen=%p port=%d framebuffer=%p",rfbScreen,rfbScreen->port,rfbScreen->frameBuffer); 
    return NGL_OK;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
    *width = rfbScreen->width;
    *height = rfbScreen->height;
    NGLOG_DEBUG("size=%dx%d",*width,*height);
    return NGL_OK;
}

DWORD nglLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    *buffer=ngs->data;
    *pitch=ngs->pitch;
    //NGLOG_DEBUG("%p buffer=%p pitch=%d",ngs,*buffer,*pitch);
    return 0;
}

DWORD nglGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format)
{
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    *width=ngs->width;
    *height=ngs->height;
    *format=GPF_ABGR;
    return NGL_OK;
}

DWORD nglUnlockSurface(HANDLE surface){
    return 0;
}

DWORD nglSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD nglFillRect(HANDLE surface,const NGLRect*rect,UINT color){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    UINT x,y;
    NGLRect recsurface={0,0,ngs->width,ngs->height};
    NGLRect *rec=(rect?rect:&recsurface);
    NGLOG_VERBOSE("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec->x,rec->y,rec->w,rec->h,color,ngs->pitch);
    UINT*fb=(UINT*)(ngs->data+ngs->pitch*rec->y+x*4);
    for(y=0;y<rec->h;y++){
        for(x=0;x<rec->w;x++)
           fb[x]=color;
        fb+=(ngs->pitch>>2);
    }
    //rfbMarkRectAsModified(rfbScreen,rec->x,rec->y,rec->x+rec->w,rec->y+rec->h);
    return NGL_OK;
}

DWORD nglFlip(HANDLE surface){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    rfbMarkRectAsModified(rfbScreen,0,0,rfbScreen->width,rfbScreen->height);
    NGLOG_VERBOSE("flip %p",ngs);
    return 0;
}

DWORD nglCreateSurface(HANDLE*surface,INT width,INT height,INT format,BOOL ishwsurface)
{//XShmCreateImage XShmCreatePixmap
     NGLSURFACE*nglsurface=(NGLSURFACE*)malloc(sizeof(NGLSURFACE));
     if(ishwsurface){
         nglsurface->data=rfbScreen->frameBuffer;
         hwsurface=nglsurface;
     }else{
         nglsurface->data=malloc(width*height*4);
     }
     nglsurface->ishw=ishwsurface;
     nglsurface->width=width;
     nglsurface->height=height;
     nglsurface->pitch=width*4;
     *surface=(HANDLE)nglsurface;
     NGLOG_DEBUG("surface=%p/%p framebuffer=%p size=%dx%d hw=%d",nglsurface,*surface,nglsurface->data,width,height,ishwsurface);
     return NGL_OK;
}

#define MIN(x,y) ((x)>(y)?(y):(x))
DWORD nglBlit(HANDLE dstsurface,NGLRect*dstrect,HANDLE srcsurface,const NGLRect*srcrect)
{
     unsigned int x,y,sx,sy,sw,sh,dx,dy;
     NGLSURFACE*ndst=(NGLSURFACE*)dstsurface;
     NGLSURFACE*nsrc=(NGLSURFACE*)srcsurface;
     UINT *pbs=(UINT*)nsrc->data;
     UINT *pbd=(UINT*)ndst->data;
     sx=srcrect?srcrect->x:0;    sy=srcrect?srcrect->y:0;
     sw=MIN(ndst->width,nsrc->width);      
     sh=MIN(ndst->height,nsrc->height);
     dx=dstrect?dstrect->x:0;    dy=dstrect?dstrect->y:0;

     NGLOG_VERBOSE("Blit from %p->%p %d,%d-%d,%d to %p %d,%d buffer=%p->%p",nsrc,ndst,sx,sy,sw,sh,ndst,dx,dy,pbs,pbd);
     pbs+=sy*(nsrc->pitch>>2)+sx;
     pbd+=dy*(ndst->pitch>>2)+dx;
     for(y=0;y<sh&&(dy+y<ndst->height);y++){
         for(x=0;x<sw;x++)pbd[x]=pbs[x];
         pbs+=(nsrc->pitch>>2);
         pbd+=(ndst->pitch>>2);
     }
     //if(ndst->ishw)rfbMarkRectAsModified(rfbScreen,0,0,rfbScreen->width,rfbScreen->height);//sx,sy,sx+sw,sy+sh);
     return 0;
}

DWORD nglDestroySurface(HANDLE surface)
{
     NGLSURFACE*ngs=(NGLSURFACE*)surface;
     if(ngs->data&&(!ngs->ishw)){
        free(ngs->data);
     }
     free(ngs);
     return 0;
}
