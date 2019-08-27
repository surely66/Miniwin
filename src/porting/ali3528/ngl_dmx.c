/*
  FILE : stub_dmx.c
  PURPOSE: This file is a stub for linking tests.
*/
#include <stdio.h>
#include "ngl_dmx.h"
#include <aui_dmx.h> 
#include <aui_tsg.h>
#include <aui_tsi.h>
#include <aui_nim.h>
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
  aui_hdl dmx;
  aui_hdl channel;
  aui_attr_dmx_channel attr;
  int num_started;
  int started;
}DMXCHANNEL;

#define TSBUF_SIZE 188*7
typedef struct{
  DMXCHANNEL*ch;
  aui_hdl hfilter;
  aui_attr_dmx_filter attr;
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

static long AuiSectionCB(aui_hdl filter_handle,unsigned char *section_data,unsigned long len,void *pv_usr_data, void *pv_reserved)
{
    NGLDMXFILTER*flt=GetFilter(filter_handle);
    if(flt&&flt->CallBack)
         flt->CallBack((DWORD)flt,(const BYTE*)section_data,len,flt->userdata); 
}
static long AuiRequestBuffer(void *p_user_hdl, unsigned long ul_req_size, void ** pp_req_buf, 
        unsigned long *req_buf_size, struct aui_avsync_ctrl *pst_ctrl_blk){
    NGLDMXFILTER*flt=GetFilter(p_user_hdl);
    *pp_req_buf  = flt->tsBuffer;
    *req_buf_size= TSBUF_SIZE;
    NGLOG_DEBUG("p_user_hdl=%p ul_req_size=%d",p_user_hdl,ul_req_size);
}

static long AuiRecvData(void *p_user_hdl, unsigned long ul_size){
    NGLDMXFILTER*flt=GetFilter(p_user_hdl);
    if(flt&&flt->CallBack)
         flt->CallBack((DWORD)flt,(const BYTE*)flt->tsBuffer,ul_size,flt->userdata);
}

static long AuiPesCBK(void *p_user_hdl,unsigned char* pbuf,unsigned long ul_size,void *usrdata){
    NGLOG_DEBUG("p_user_hdl=%p pbuf=%p/%02x ul_size=%d ",p_user_hdl,pbuf,pbuf[0],ul_size);
    NGLDMXFILTER*flt=GetFilter(p_user_hdl);
    if(flt&&flt->CallBack)
        flt->CallBack((DWORD)flt,pbuf,ul_size,flt->userdata); 
}

static void TSInjectProc(void*p)
{//tsg will caused higher cpu usage above 50%
    unsigned char buffer[256*188];
    char*tsgfile=getenv("TSINJECT_FILE");
    FILE *file = fopen(tsgfile, "rb");
    struct aui_attr_tsg attr;
    aui_hdl tsg_hdl=(aui_hdl)p;
    bzero(&attr,sizeof(attr));
    NGLOG_DEBUG("TSInjectProc $TSINJECT_FILE=%s file=%p  tsg_hdl=%p",tsgfile,file,tsg_hdl);
    if(file==NULL)return;
    while(1){
        unsigned long pkt_in_tsg;
        int cnt=fread(buffer,188,256,file);
        attr.ul_addr = (unsigned char *)buffer;
        attr.ul_pkg_cnt = cnt;
        attr.uc_sync_mode = 1;//AUI_TSG_XFER_SYNC;
        int ret = aui_tsg_send(tsg_hdl, &attr);
        if(ret!=0)NGLOG_DEBUG("aui_tsg_send failed");
        if(feof(file))fseek(file,0,SEEK_SET);
    }
}
struct aui_tsi_config {
	unsigned long ul_hw_init_val;
	unsigned long ul_input_src;
};

static int aui_tsi_config(struct aui_tsi_config *tsi_cfg);
#define AUI_TSG_DEV	AUI_NIM_HANDLER_MAX
#define MAX_TSI_DEVICE	(AUI_TSG_DEV + 1)
DWORD nglDmxInit(){
    int i;
    DWORD thid;
    static int sDMX_INITED=0;
    struct aui_attr_tsg attr_tsg;
    struct aui_attr_tsi attr_tsi;
    aui_hdl tsg_hdl;
    aui_hdl tsi_hdl;
    if(mtx_dmx)return 0;
    NGLOG_DEBUG("nglDmxInit");
    aui_dmx_init(NULL,NULL);
    aui_log_priority_set(AUI_MODULE_DMX,AUI_LOG_PRIO_ERR);
    nglCreateMutex(&mtx_dmx);
    
    for(i=0;i<MAX_CHANNEL;i++){
        Channels[i].dmx=NULL;
        Channels[i].channel=NULL;
        Channels[i].pid=UNUSED_PID;
        Channels[i].num_filt=0;
    }
#define MAX_TSI_DEVICE 4 
#define MAX_TSI_CNT 4
    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];/* If use MAX_TSI_CNT, array bounds will happen */
    enum aui_tsi_channel_id tsi_channel_id[MAX_TSI_CNT] = {AUI_TSI_CHANNEL_0, AUI_TSI_CHANNEL_1, AUI_TSI_CHANNEL_2, AUI_TSI_CHANNEL_3};
    aui_tsi_output_id dmx_id[MAX_TSI_CNT] = {AUI_TSI_OUTPUT_DMX_0,    AUI_TSI_OUTPUT_DMX_1, AUI_TSI_OUTPUT_DMX_2,   AUI_TSI_OUTPUT_DMX_3};
    
    MEMSET(&attr_tsi, 0, sizeof(aui_attr_tsi));
    MEMSET(tsi_cfg, 0xff, sizeof(tsi_cfg));  /* not 0x00 */
     /* open tsi */
    if(aui_find_dev_by_idx(AUI_MODULE_TSI, 0, &tsi_hdl)) {
        if (aui_tsi_open(&tsi_hdl)) {
            goto EXIT_FAIL;
        }
        NGLOG_DEBUG("tsi_hdl=%p",tsi_hdl);
    }
   
    /* config tsi */
    aui_tsi_config(tsi_cfg);
    for (i = 0; i<MAX_TSI_CNT && i < AUI_NIM_HANDLER_MAX; i++) {
        NGLOG_DEBUG("tsi_cfg[%d].ul_input_src=%d<-->%d",i,tsi_cfg[i].ul_input_src,AUI_TSI_INPUT_MAX);
        if (AUI_TSI_INPUT_MAX > tsi_cfg[i].ul_input_src) {
	    if(tsi_cfg[i].ul_hw_init_val==0)break;
			
            attr_tsi.ul_init_param = tsi_cfg[i].ul_hw_init_val;
            int rc=aui_tsi_src_init(tsi_hdl, tsi_cfg[i].ul_input_src, &attr_tsi);
            NGLOG_DEBUG("aui_tsi_src_init=%d",rc);
           
            rc=aui_tsi_route_cfg(tsi_hdl,tsi_cfg[i].ul_input_src, tsi_channel_id[i],dmx_id[i]);
            NGLOG_DEBUG("aui_tsi_route_cfg=%d",rc);
        }
    }
 
#if 1 
    NGLOG_DEBUG("tsi_hdl=%p",tsi_hdl);

    aui_tsg_init(NULL,NULL);

    bzero(&attr_tsg,sizeof(aui_attr_tsg));
    attr_tsg.ul_tsg_clk = 24;
    attr_tsg.ul_bit_rate = 100;
    aui_tsg_open(&attr_tsg,&tsg_hdl);
    aui_tsg_start(tsg_hdl,&attr_tsg);
    //aui_tsi_open(&tsi_hdl);
    NGLOG_DEBUG("tsg_hdl=%p",tsg_hdl);
    attr_tsi.ul_init_param = AUI_TSI_IN_CONF_ENABLE |AUI_TSI_IN_CONF_VALID_SIG_POL |AUI_TSI_IN_CONF_SYNC_SIG_POL;
    aui_tsi_src_init(tsi_hdl,AUI_TSI_INPUT_TSG,&attr_tsi);
    aui_tsi_route_cfg(tsi_hdl,AUI_TSI_INPUT_TSG,AUI_TSI_CHANNEL_1,AUI_TSI_OUTPUT_DMX_1);
    nglCreateThread(&thid,0,4096,TSInjectProc,tsg_hdl);
#endif
   return 0;
EXIT_FAIL:
   return -1;
}

DWORD nglAllocateSectionFilter(INT dmx_id,WORD  wPid,NGL_DMX_FilterNotify cbk,void*userdata,NGL_DMX_TYPE dmxtype)
{
    int rc;
    aui_attr_dmx dmxattr;
    aui_attr_dmx_filter fltattr;
    aui_hdl hdl_filter;
    aui_dmx_data_type dmxtype2aui[]={AUI_DMX_DATA_RAW,AUI_DMX_DATA_SECT,AUI_DMX_DATA_PES};
    bzero(&dmxattr,sizeof(dmxattr));
    if(NULL==cbk)
       return 0;
    nglLockMutex(mtx_dmx);
    DMXCHANNEL*ch=GetChannel(wPid);
    if(ch==NULL){
        ch=GetChannel(UNUSED_PID);
        ch->pid=wPid;
        ch->num_filt=0;
        if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX, dmx_id, &ch->dmx)){
            dmxattr.uc_dev_idx=dmx_id;
            CHECKDMX(aui_dmx_open(&dmxattr,&ch->dmx));
            CHECKDMX(aui_dmx_start(ch->dmx,&dmxattr));
        }
        bzero(&ch->attr,sizeof(aui_attr_dmx_channel));
        ch->attr.us_pid = wPid;
        ch->attr.dmx_data_type =dmxtype2aui[dmxtype];
        rc=aui_dmx_channel_open(ch->dmx, &ch->attr, &ch->channel);
        NGLOG_VERBOSE("aui_dmx_channel_open=%d channel=%p",rc,ch->channel);
    };
    NGLDMXFILTER*flt=GetFilter(NULL);
    flt->ch=ch;
    NGLOG_VERBOSE("dmx:%d pid:%d flt=%p ch=%p ch.channel=%p",dmx_id,wPid,flt,ch,(ch?ch->channel:NULL));
    bzero(&flt->attr,sizeof(flt->attr));
    flt->attr.puc_mask=flt->mask;
    flt->attr.puc_val=flt->value;
    switch(dmxtype){
    case NGL_DMX_SECTION:flt->attr.p_fun_sectionCB=AuiSectionCB;break;
    case NGL_DMX_PES:flt->attr.callback=AuiPesCBK;             break;
    case NGL_DMX_TS :flt->attr.p_fun_data_req_wtCB=AuiRequestBuffer;
                     flt->attr.p_fun_data_up_wtCB =AuiRecvData; 
                     flt->tsBuffer=(BYTE*)malloc(TSBUF_SIZE);break;
    }
    CHECKDMX(aui_dmx_filter_open(ch->channel,&flt->attr,&flt->hfilter));
    /*if(NGL_DMX_SECTION!=dmxtype){
        aui_dmx_reg_data_call_back(flt->hfilter, AuiBufReq,AuiPesEsCB);
    }else
        CHECKDMX(aui_dmx_reg_sect_call_back(flt->hfilter,AuiSectionCB));*/

    NGLOG_VERBOSE("dmxid=%d dmx=%p filter=%p pid=%x flthandle=%p ch=%p/%p",dmx_id,ch->dmx,flt,wPid,flt->hfilter,ch,ch->channel);
    ch->num_filt++;
    flt->CallBack=cbk;
    flt->userdata=userdata;
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
  CHECKFILTER(flt);
  nglLockMutex(mtx_dmx);
  if(flt->tsBuffer)free(flt->tsBuffer);
  flt->tsBuffer=NULL;
  if(flt->ch){
      flt->ch->num_filt--;
      CHECKDMX(aui_dmx_filter_close(&(flt->hfilter)));
      if(flt->ch->num_filt==0){
           int rc=aui_dmx_channel_close(&flt->ch->channel);
           NGLOG_VERBOSE("aui_dmx_channel_close=%d",rc);
      }
      flt->ch=NULL;
  }else{
      nglUnlockMutex(mtx_dmx);
      return NGL_ERROR;
  }
  flt->hfilter=NULL;
  nglUnlockMutex(mtx_dmx);
  return NGL_OK;
}

INT nglSetSectionFilterParameters( DWORD dwStbFilterHandle,UINT uiLength, BYTE *pValue, BYTE *pMask)
{
  BYTE reverse[16];
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  CHECKFILTER(flt);
  if(uiLength==0||pValue==NULL||pMask==0)
      return NGL_INVALID_PARA;
  bzero(reverse,sizeof(reverse));
  memcpy(flt->mask,pMask,uiLength);
  memcpy(flt->value,pValue,uiLength);
  
  flt->attr.ul_mask_val_len=uiLength;
  flt->attr.uc_crc_check=0;
  flt->attr.uc_continue_capture_flag=1;//(eNotificationMode==eNGL_DMX_Continuous);
  CHECKDMX(aui_dmx_filter_mask_val_cfg(flt->hfilter,pMask,pValue,reverse,uiLength,0,1));
  return NGL_OK;
}

INT nglStartSectionFilter(DWORD  dwStbFilterHandle)
{
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  CHECKFILTER(flt);
  nglLockMutex(mtx_dmx);
  if(flt->started==0){
     if(flt->ch->started==0){
         CHECKDMX(aui_dmx_channel_start(flt->ch->channel,&flt->ch->attr));
         flt->ch->started++;
     }
     CHECKDMX(aui_dmx_filter_start(flt->hfilter,&flt->attr));
     flt->started++;
     flt->ch->num_started++;
  }
  nglUnlockMutex(mtx_dmx);
  return NGL_OK;
}

/**AllocSectionFilter without FreeSectionFilter ?*/
INT nglStopSectionFilter(DWORD  dwStbFilterHandle)
{
  NGLOG_VERBOSE("NGLStopSectionFilter filter=0x%x",dwStbFilterHandle);
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  CHECKFILTER(flt);
  nglLockMutex(mtx_dmx);
  if(flt->started){
     CHECKDMX(aui_dmx_filter_stop(flt->hfilter,&flt->attr));
     aui_dmx_reg_data_call_back(flt->hfilter,NULL,NULL);
     aui_dmx_reg_sect_call_back(flt->hfilter,NULL);
     flt->started--;
     flt->ch->num_started--;
     if(flt->ch->num_started==0){
          int rc=aui_dmx_channel_stop(flt->ch->channel,&flt->ch->attr);
          NGLOG_VERBOSE("aui_dmx_channel_stop=%d",rc);
     }
  }
  nglUnlockMutex(mtx_dmx);
  return NGL_OK;
}

INT nglUnlockSectionFilter(DWORD dwStbFilterHandle)
{
  NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
  CHECKFILTER(flt);
  return NGL_OK;
}
#define BOARD_CFG_M3528 1
#define INC_INDEX(index) { index++; if (index == AUI_NIM_HANDLER_MAX) return AUI_RTN_FAIL; }
static int aui_tsi_config(struct aui_tsi_config *tsi_cfg)
{
	int index = 0;
	struct aui_tsi_config *cfg = tsi_cfg;

	/* TSI configuration for NIM 0 & 1 */
#if (defined BOARD_CFG_M3515)
#if (defined AUI_BOARD_VERSION_01V04)
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;
#else
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;
#endif
#elif (defined BOARD_CFG_M3715B)
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x9b (1001 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

#elif  (defined BOARD_CFG_M3515B)

	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	/*
	  cfg[1].ul_hw_init_val =  // 0x9b does not work
	  AUI_TSI_IN_CONF_SPI_ENABLE
		| AUI_TSI_IN_CONF_SSI_CLOCK_POL
		| AUI_TSI_IN_CONF_SSI_BIT_ORDER
		| AUI_TSI_IN_CONF_SYNC_SIG_POL
		| AUI_TSI_IN_CONF_VALID_SIG_POL;
	*/
	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;

	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

#elif (defined BOARD_CFG_M3733)
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x15f;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x2f (0010 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x2f (0010 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_1;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x2f (0010 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_3;
#elif (defined BOARD_CFG_M3755)
    //ul_hw_init_val: 0x8b (1000 1011)
    cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
            | AUI_TSI_IN_CONF_SSI_BIT_ORDER
            | AUI_TSI_IN_CONF_SYNC_SIG_POL
            | AUI_TSI_IN_CONF_VALID_SIG_POL;
    cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

    INC_INDEX(index); cfg++;
    //ul_hw_init_val: 0x8b (1000 1011)
    cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
            | AUI_TSI_IN_CONF_SSI_BIT_ORDER
            | AUI_TSI_IN_CONF_SYNC_SIG_POL
            | AUI_TSI_IN_CONF_VALID_SIG_POL;
    cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

#ifdef CONFIG_ALI_EMULATOR    
	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	#if 1
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_1;
	#else
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_1;
	#endif
#endif    
	
#elif (defined BOARD_CFG_M3823)
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;

#elif (defined BOARD_CFG_M3735)
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_1;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_0;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;
#elif (defined BOARD_CFG_M3527)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_1;
#elif (defined BOARD_CFG_M3528)
        NGLOG_DEBUG("BOARD_CFG_M3528");
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5f (0101 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_3;
#elif (defined BOARD_CFG_M3529)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_2;
#elif (defined BOARD_CFG_M3626)
    cfg->ul_hw_init_val = 0x27;
    cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;
#elif (defined BOARD_CFG_M3627)
#ifdef SUPPORT_TWO_TUNER
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
#endif
	index = index;  //avoid compilation errors
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_1;
#elif (defined BOARD_CFG_M3727)||(defined BOARD_CFG_M3712)||(defined BOARD_CFG_M3712L)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;
#elif (defined BOARD_CFG_M3716L)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
	               | AUI_TSI_IN_CONF_SSI_BIT_ORDER
	               | AUI_TSI_IN_CONF_SYNC_SIG_POL
	               | AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;
	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
	               | AUI_TSI_IN_CONF_SSI_BIT_ORDER
	               | AUI_TSI_IN_CONF_SYNC_SIG_POL
	               | AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;
#endif
	/* TSI configuration for TSG */
	//ul_hw_init_val: 0x83 (1000 0011)
	tsi_cfg[AUI_TSG_DEV].ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	tsi_cfg[AUI_TSG_DEV].ul_input_src = AUI_TSI_INPUT_TSG;

	return 0;
};

/* End of File */
