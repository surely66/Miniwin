/*
  FILE : stub_dmx.c
  PURPOSE: This file is a stub for linking tests.
*/
#include <stdio.h>
#include "ngl_dmx.h"
#include <strings.h>
#include <av/gxav_demux_propertytypes.h>
#include <av/gxav_common.h>
#include <gxcore.h>
#include <bus/gxavdev.h>

#define MASK_LEN 16 
#define MAX_CHANNEL 64
#define MAX_FILTER  128 
#define UNUSED_PID -1
#include <ngl_log.h>
#include <ngl_os.h>

NGL_MODULE(DMX)

#define CHECKDMX(x) {if((x)!=AUI_RTN_SUCCESS)NGLOG_VERBOSE("%s:%d %s=%d\n",__FUNCTION__,__LINE__,#x,(x));}
typedef struct{
   int dev;
   int hdl;
}GXDMX;
typedef struct {
  unsigned short pid;
  int num_filt;
  int num_started;
  int started;
  int dmxid;
  int slot_id;
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
  int filt_id;
}NGLDMXFILTER;
static NGLMutex mtx_dmx=0;
static GXDMX sDMX[2];
static DMXCHANNEL Channels[MAX_CHANNEL];
static NGLDMXFILTER  Filters[MAX_FILTER];
static DMXCHANNEL*GetChannel(int pid){
   int i;
   for(i=0;i<MAX_CHANNEL;i++){
       DMXCHANNEL*ch=Channels+i;
       if((ch->pid==pid)||(pid>=0x1FFF&&ch->pid>=0x1FFF)){
            return Channels+i;
       }
   }
   return NULL;
}

static NGLDMXFILTER*GetFilter(DMXCHANNEL*ch){
    int i;
    for(i=0;i<MAX_FILTER;i++)
        if(Filters[i].ch==NULL){
            Filters[i].ch=ch;
            return Filters+i;
        }
    return NULL;
}
#define CHECKFILTER(flt) {if((flt<Filters)||(flt>=&Filters[MAX_FILTER]))return NGL_INVALID_PARA;}

DWORD nglDmxInit(){
    int i,dev;
    DWORD thid;
    if(mtx_dmx)return 0;
    NGLOG_DEBUG("nglDmxInit\r\n");
    nglCreateMutex(&mtx_dmx);
    dev=GxAvdev_CreateDevice(0);
    for(i=0;i<2;i++){
        GxDemuxProperty_ConfigDemux cfg;
        sDMX[i].dev=dev;
        sDMX[i].hdl=GxAvdev_OpenModule(dev, GXAV_MOD_DEMUX, i);
        cfg.source = i;
        cfg.ts_select = FRONTEND;
        cfg.stream_mode = DEMUX_PARALLEL;
        cfg.time_gate = 0xf;
        cfg.byt_cnt_err_gate = 0x03;
        cfg.sync_loss_gate = 0x03;
        cfg.sync_lock_gate = 0x03;
        GxAVSetProperty(dev,sDMX[i].hdl,GxDemuxPropertyID_Config,&cfg,sizeof(GxDemuxProperty_ConfigDemux));
    }
    for(i=0;i<MAX_CHANNEL;i++){
        //Channels[i].dmx=NULL;
        //Channels[i].channel=NULL;
        Channels[i].pid=UNUSED_PID;
        Channels[i].num_filt=0;
    }
   return 0;
}

static int GetProps(const NGLDMXFILTER*flt,GxDemuxProperty_Slot*sp,GxDemuxProperty_Filter*fp){
    int rc=0;
    int dmxid=flt->ch->dmxid;
    memset(sp,0,sizeof(GxDemuxProperty_Slot));
    memset(fp,0,sizeof(GxDemuxProperty_Filter));
    sp->slot_id=flt->ch->slot_id;
    fp->slot_id=flt->ch->slot_id;
    fp->filter_id=flt->filt_id;
    rc=GxAVGetProperty(sDMX[dmxid].dev,sDMX[dmxid].hdl,GxDemuxPropertyID_SlotAlloc,(void*)sp, sizeof(GxDemuxProperty_Slot));
    if(rc>=0){
        fp->filter_id=flt->filt_id;
        rc=GxAVGetProperty(sDMX[dmxid].dev,sDMX[dmxid].hdl,GxDemuxPropertyID_FilterAlloc,(void*)fp, sizeof(GxDemuxProperty_Filter));
    }     
    return rc;
}

HANDLE nglAllocateSectionFilter(INT id,WORD  wPid,NGL_DMX_FilterNotify cbk,void*userdata,NGL_DMX_TYPE dmxtype)
{
    int rc=0;
    GxDemuxProperty_Slot slot_prop;
    GxDemuxProperty_Filter filt_prop;
    DMXCHANNEL*ch=GetChannel(wPid);
    NGLDMXFILTER*flt=GetFilter(ch);
    nglLockMutex(mtx_dmx);
    memset(&slot_prop, 0, sizeof(GxDemuxProperty_Slot));
    memset(&filt_prop, 0, sizeof(GxDemuxProperty_Filter));
    if(NULL==ch){
        ch=GetChannel(0xFFFF);//alloc new channel
        slot_prop.type = DEMUX_SLOT_PSI;
        slot_prop.pid = wPid;
        rc=GxAVGetProperty(sDMX[id].dev,sDMX[id].hdl,GxDemuxPropertyID_SlotAlloc,(void*)&slot_prop, sizeof(GxDemuxProperty_Slot));

        ch->slot_id=slot_prop.slot_id;
        ch->pid=wPid;
        slot_prop.type = DEMUX_SLOT_PSI;
        slot_prop.flags= (DMX_REPEAT_MODE | DMX_AVOUT_EN);
        rc=GxAVSetProperty(sDMX[id].dev,sDMX[id].hdl, GxDemuxPropertyID_SlotConfig,(void*)&slot_prop, sizeof(GxDemuxProperty_Slot));
    }
    flt->ch=ch;
    ch->num_filt++;
    if(rc>=0){
        filt_prop.slot_id = slot_prop.slot_id;
        rc=GxAVGetProperty(sDMX[id].dev,sDMX[id].hdl,GxDemuxPropertyID_FilterAlloc,(void*)&filt_prop, sizeof(GxDemuxProperty_Filter));
    }
    if(rc<0){
        GxAVSetProperty(sDMX[id].dev,sDMX[id].hdl, GxDemuxPropertyID_SlotFree,(void*)&slot_prop, sizeof(GxDemuxProperty_Slot));
        nglUnlockMutex(mtx_dmx);
        return NULL;
    }
    flt->ch=ch;
    flt->filt_id=filt_prop.filter_id;
    nglUnlockMutex(mtx_dmx);
    return flt; 
}

INT nglGetFilterPid( HANDLE dwStbFilterHandle){
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    return flt->ch->pid;
}

INT nglFreeSectionFilter( HANDLE dwStbFilterHandle )
{
    GxDemuxProperty_Filter filt_prop;
    GxDemuxProperty_Slot slot_prop;
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    nglLockMutex(mtx_dmx);
    if(flt->ch){
        GetProps(flt,&slot_prop,&filt_prop);
        GxAVSetProperty(sDMX[flt->ch->dmxid].dev, sDMX[flt->ch->dmxid].hdl, GxDemuxPropertyID_FilterFree,
               (void*)&filt_prop, sizeof(GxDemuxProperty_Filter));
        flt->ch->num_filt--;
        if(flt->ch->num_filt==0){
            GxAVSetProperty(sDMX[flt->ch->dmxid].dev,sDMX[flt->ch->dmxid].hdl,GxDemuxPropertyID_SlotFree,
                    (void*)&slot_prop, sizeof(GxDemuxProperty_Slot));
            flt->ch->pid=0xFFFF;
        }
        flt->ch=NULL;
    }
    nglUnlockMutex(mtx_dmx);
    return NGL_OK;
}

INT nglSetSectionFilterParameters( HANDLE dwStbFilterHandle, BYTE *pMask, BYTE *pValue,UINT uiLength)
{
    int i;
    BYTE reverse[16];
    GxDemuxProperty_Slot slot_prop;
    GxDemuxProperty_Filter filt_prop;
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    CHECKFILTER(flt);
    GetProps(flt,&slot_prop,&filt_prop);
    filt_prop.depth=uiLength;
    for(i=0;i<uiLength;i++){
         filt_prop.key[i].value=pValue[i];
         filt_prop.key[i].mask=pMask[i];
    }
    GxAVSetProperty(sDMX[flt->ch->dmxid].dev, sDMX[flt->ch->dmxid].hdl, GxDemuxPropertyID_FilterConfig,
         (void*)&filt_prop, sizeof(GxDemuxProperty_Filter)); 
    return NGL_OK;
}

INT nglStartSectionFilter(HANDLE  dwStbFilterHandle)
{
    GxDemuxProperty_Slot slot_prop;
    GxDemuxProperty_Filter filt_prop;
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    CHECKFILTER(flt);
    GetProps(flt,&slot_prop,&filt_prop);
    nglLockMutex(mtx_dmx);
    if(flt->started==0 && flt->ch->num_started==0){
        GxAVSetProperty(sDMX[flt->ch->dmxid].dev, sDMX[flt->ch->dmxid].hdl, GxDemuxPropertyID_SlotEnable,
              (void*)&slot_prop,sizeof(GxDemuxProperty_Slot));
    }
    GxAVSetProperty(sDMX[flt->ch->dmxid].dev,sDMX[flt->ch->dmxid].hdl, GxDemuxPropertyID_FilterEnable,
                            (void*)&filt_prop, sizeof(GxDemuxProperty_Filter));
    if(!flt->started){
        flt->started=1;
        flt->ch->num_started++;
    }
    nglUnlockMutex(mtx_dmx);
    return NGL_OK;
}

/**AllocSectionFilter without FreeSectionFilter ?*/
INT nglStopSectionFilter(HANDLE dwStbFilterHandle)
{
    NGLOG_VERBOSE("NGLStopSectionFilter filter=0x%x",dwStbFilterHandle);
    GxDemuxProperty_Slot slot_prop;
    GxDemuxProperty_Filter filt_prop;
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    CHECKFILTER(flt);
    GetProps(flt,&slot_prop,&filt_prop);
    nglLockMutex(mtx_dmx);
    if(flt->started){
        GxAVSetProperty(sDMX[flt->ch->dmxid].dev,sDMX[flt->ch->dmxid].hdl, GxDemuxPropertyID_FilterDisable,
                            (void*)&filt_prop, sizeof(GxDemuxProperty_Filter));
        flt->started--;
        flt->ch->num_started--;
        if(flt->ch->num_started==0){
            GxAVSetProperty(sDMX[flt->ch->dmxid].dev, sDMX[flt->ch->dmxid].hdl, GxDemuxPropertyID_SlotDisable,
                (void*)&slot_prop,sizeof(GxDemuxProperty_Slot));
        }
    }
    return NGL_OK;
}

INT nglUnlockSectionFilter(HANDLE dwStbFilterHandle)
{
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    CHECKFILTER(flt);
    return NGL_OK;
}

/* End of File */
