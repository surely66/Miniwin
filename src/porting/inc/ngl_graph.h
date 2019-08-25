#ifndef __NGL_GRAPH_H__
#define __NGL_GRAPH_H__
#include <ngl_types.h>

NGL_BEGIN_DECLS
typedef enum {
  GPF_UNKNOWN,
  GPF_ARGB4444,
  GPF_ARGB1555,
  GPF_ARGB,
  GPF_RGB32
}NGLPIXELFORMAT;

typedef struct{
    INT x;
    INT y;
    INT w;
    INT h;
}NGLRect;

DWORD nglGraphInit();
DWORD nglGetScreenSize(UINT*width,UINT*height);
DWORD nglCreateSurface(DWORD*surface,INT width,INT height,INT format,BOOL hwsurface);
DWORD nglGetSurfaceInfo(DWORD surface,UINT*width,UINT*height,INT *format);
DWORD nglLockSurface(DWORD surface,void**buffer,UINT*pitch);
DWORD nglUnlockSurface(DWORD surface);
DWORD nglSurfaceSetOpacity(DWORD surface,BYTE alpha);
/*
  rect :if NULL fill whole surface area
  color:A8R8G8B8
*/
DWORD nglFillRect(DWORD dstsurface,const NGLRect*rect,UINT color);
DWORD nglBlit(DWORD dstsurface,DWORD srcsurface,const NGLRect*srcrect,const NGLRect*dstrect);
DWORD nglFlip(DWORD dstsurface);
DWORD nglDestroySurface(DWORD surface);

NGL_END_DECLS
#endif

