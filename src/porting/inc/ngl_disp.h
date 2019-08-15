#ifndef __NGL_DISP_H__
#define __NGL_DISP_H__

NGL_BEGIN_DECLS
enum{
   DISP_APR_AUTO,
   DISP_APR_4_3,
   DISP_APR_16_9
}DISP_ASPECT_RATIO;

enum{
   DISP_MM_PANSCAN,
   DISP_MM_LETTERBOX,
   DISP_MM_PILLBOX,
   DISP_MM_NORMAL_SCALE,
   DISP_COMBINED_SCALE
};

DWORD nglDispInit();
DWORD nglDispSetAspectRatio(int ratio);
DWORD nglDispGetAspectRatio(int *ratio);
DWORD nglDispSetMatchMode(int md);

NGL_END_DECLS

#endif
