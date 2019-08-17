/*
  FILE : stub_dmx.c
  PURPOSE: This file is a stub for linking tests.
*/
#include <stdio.h>
#include "ngl_dmx.h"
#include <strings.h>

#define MASK_LEN 16 
#define MAX_CHANNEL 64
#define MAX_FILTER  128 
#define UNUSED_PID -1
#include <ngl_log.h>
#include <ngl_os.h>

NGL_MODULE(DMX)

#define CHECKDMX(x) {if((x)!=AUI_RTN_SUCCESS)NGLOG_VERBOSE("%s:%d %s=%d\n",__FUNCTION__,__LINE__,#x,(x));}
typedef struct {
  int pid;
  int num_filt;
  int num_started;
  int started;
}DMXCHANNEL;

#define TSBUF_SIZE 188*7
typedef struct{
  DMXCHANNEL*ch;
  NGL_DMX_FilterNotify CallBack;
  void*userdata; 
  int started;
  BYTE mask[MASK_LEN+2];
  BYTE value[MASK_LEN+2];
  BYTE reverse[MASK_LEN+2];
  BYTE*tsBuffer;
}NGLDMXFILTER;
static NGLMutex mtx_dmx=0;
static DMXCHANNEL Channels[MAX_CHANNEL];
static NGLDMXFILTER  Filters[MAX_FILTER];
static DMXCHANNEL*GetChannel(int pid){
  int i;
  for(i=0;i<MAX_CHANNEL;i++){
     if(Channels[i].pid==pid)
         return Channels+i;
  }
  return NULL;
}
#define CHECKFILTER(flt) {if((flt<Filters)||(flt>=&Filters[MAX_FILTER]))return NGL_INVALID_PARA;}
static NGLDMXFILTER*GetFilter(aui_hdl hdl){
   int i;
   for(i=0;i<MAX_FILTER;i++){
       if(Filters[i].hfilter==hdl)
          return Filters+i;
   }
   return NULL;
}

DWORD nglDmxInit(){
    int i;
    DWORD thid;
    static int sDMX_INITED=0;
    struct aui_attr_tsg attr_tsg;
    struct aui_attr_tsi attr_tsi;
    if(mtx_dmx)return 0;
    NGLOG_DEBUG("nglDmxInit\r\n");
    nglCreateMutex(&mtx_dmx);
    
    for(i=0;i<MAX_CHANNEL;i++){
        Channels[i].dmx=NULL;
        Channels[i].channel=NULL;
        Channels[i].pid=UNUSED_PID;
        Channels[i].num_filt=0;
    }
   return 0;
}

DWORD nglAllocateSectionFilter(INT dmx_id,WORD  wPid,NGL_DMX_FilterNotify cbk,void*userdata,NGL_DMX_TYPE dmxtype)
{
    int rc;
    nglLockMutex(mtx_dmx);
    nglUnlockMutex(mtx_dmx);
    return (DWORD)flt;
}

INT nglGetFilterPid( DWORD dwStbFilterHandle){
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  return flt->ch?flt->ch->pid:-1;
}
INT nglFreeSectionFilter( DWORD dwStbFilterHandle )
{
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  nglLockMutex(mtx_dmx);
  nglUnlockMutex(mtx_dmx);
  return NGL_OK;
}

INT nglSetSectionFilterParameters( DWORD dwStbFilterHandle,UINT uiLength, BYTE *pValue, BYTE *pMask)
{
  BYTE reverse[16];
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  CHECKFILTER(flt);
  return NGL_OK;
}

INT nglStartSectionFilter(DWORD  dwStbFilterHandle)
{
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  CHECKFILTER(flt);
  nglLockMutex(mtx_dmx);
  nglUnlockMutex(mtx_dmx);
  return NGL_OK;
}

/**AllocSectionFilter without FreeSectionFilter ?*/
INT nglStopSectionFilter(DWORD  dwStbFilterHandle)
{
  NGLOG_VERBOSE("NGLStopSectionFilter filter=0x%x",dwStbFilterHandle);
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  nglLockMutex(mtx_dmx);
  nglUnlockMutex(mtx_dmx);
  return NGL_OK;
}

INT nglUnlockSectionFilter(DWORD dwStbFilterHandle)
{
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  CHECKFILTER(flt);
  return NGL_OK;
}

/* End of File */
