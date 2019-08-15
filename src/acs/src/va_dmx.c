/*
  FILE : stub_dmx.c
  PURPOSE: This file is a stub for linking tests.
*/
#include <stdio.h>
#include "va_dmx.h"
#include <ngl_dmx.h>
#include "va_errors.h"
#include <strings.h>
#include <ngl_log.h>
NGL_MODULE(ACSDMX)

#define MASK_LEN 8
#define MAX_CHANNEL 32
#define MAX_FILTER  64
#define UNUSED_PID -1

#define CHECK(x) x //{/*if((x)!=AUI_RTN_SUCCESS)*/NGLOG_DEBUG("%s:%d %s=%d\n",__FUNCTION__,__LINE__,#x,(x));}

typedef enum{
   FILTER_INITED=0,
   FILTER_STARTED=1,
   FILTER_STOPPED=2,
}FILTERSTAT;
typedef struct{
  DWORD handle;
  tVA_DMX_NotifyFilteredSection CallBack;
  FILTERSTAT state;
}DMXFILTER;

DMXFILTER  Filters[MAX_FILTER];

static void NGLSectionCB(DWORD filter_handle,const BYTE *data,UINT len,void*userdata)
{
    DMXFILTER*flt=(DMXFILTER*)userdata;
    if(NULL!=flt->CallBack)
        flt->CallBack((DWORD)flt,len,(BYTE*)data,NULL); 
}

void VA_DMX_Init(){
   int i;
   static int sDMX_INITED=0;
   if(sDMX_INITED!=0)
      return;
   nglDmxInit();
   for(i=0;i<MAX_FILTER;i++){
       Filters[i].handle=0;
       Filters[i].CallBack=NULL;
   }
   sDMX_INITED++;
}

DWORD VA_DMX_AllocateSectionFilter(DWORD dwAcsId, DWORD dwVaFilterHandle, WORD  wPid,
                           tVA_DMX_NotifyFilteredSection pfVA_DMX_NotifyFilteredSection)
{
  int i;
  VA_DMX_Init();
  DMXFILTER*flt=NULL;
  for(i=0;i<MAX_FILTER;i++){
      if(0==Filters[i].handle){
          flt=Filters+i; 
          break;
      }
  }
  flt->state=FILTER_INITED;
  flt->CallBack=pfVA_DMX_NotifyFilteredSection;
  flt->handle=nglAllocateSectionFilter(0,wPid,NGLSectionCB,(void*)flt,NGL_DMX_SECTION);
  NGLOG_DEBUG("filter=%p handle=%p pid=0x%x/%d\n",flt,flt->handle,wPid,wPid);
  return (DWORD)flt;
}

#define CHECKFILTER(s) {if(s<Filters||s>=& Filters[MAX_FILTER])return kVA_INVALID_PARAMETER;}

INT VA_DMX_FreeSectionFilter( DWORD dwStbFilterHandle )
{
  DMXFILTER*flt=(DMXFILTER*)dwStbFilterHandle;
  NGLOG_DEBUG("flt=%p",flt);
  CHECKFILTER(flt);
  if(FILTER_INITED!=flt->state && FILTER_STARTED!=flt->state)
      return kVA_STOP_FILTER_FIRST;
  flt->state=FILTER_INITED;
  nglFreeSectionFilter(flt->handle);
  flt->handle=0;
  return 0;
}

/*#ifdef VAOPT_ENABLE_ACS41
INT VA_DMX_SetSectionFilterParameters(
                                      DWORD dwStbFilterHandle, 
                                      UINT32 uiLength, BYTE *pValue, BYTE *pMask)
#else*/
INT VA_DMX_SetSectionFilterParameters(DWORD dwStbFilterHandle,
                                      UINT32 uiLength, BYTE *pValue, BYTE *pMask,
                                      tVA_DMX_NotificationMode eNotificationMode)
//#endif
{
  DMXFILTER*flt=(DMXFILTER*)dwStbFilterHandle;
  CHECKFILTER(flt);
  if( kVA_DMX_MAX_FILTER_SIZE<uiLength || NULL==pValue|| NULL==pMask)
      return kVA_INVALID_PARAMETER;
  nglSetSectionFilterParameters(flt->handle,uiLength,pValue,pMask);
  return 0;
}

INT VA_DMX_StartSectionFilter(DWORD  dwStbFilterHandle)
{
  DMXFILTER*flt=(DMXFILTER*)dwStbFilterHandle;
  NGLOG_DEBUG("flt=%p",flt);
  CHECKFILTER(flt);
  flt->state=FILTER_STARTED;
  nglStartSectionFilter(flt->handle);
  return 0;
}

/**AllocSectionFilter without FreeSectionFilter ?*/
INT VA_DMX_StopSectionFilter(DWORD  dwStbFilterHandle)
{
  DMXFILTER*flt=(DMXFILTER*)dwStbFilterHandle;
  NGLOG_DEBUG("flt=%p",flt);
  CHECKFILTER(flt);
  flt->state=FILTER_STOPPED;
  nglStopSectionFilter(flt->handle);
  return 0;
}

#if (defined VAOPT_ENABLE_ACS30 || defined VAOPT_ENABLE_ACS31 || defined VAOPT_ENABLE_ACS40 || defined VAOPT_ENABLE_ACS41)
INT VA_DMX_UnlockSectionFilter(DWORD dwStbFilterHandle)
{
  DMXFILTER*flt=(DMXFILTER*)dwStbFilterHandle;
  NGLOG_DEBUG("flt=%p",flt);
  CHECKFILTER(flt);
  return 0;
}
#endif

/* End of File */
