#include<aui_dsc.h>
#include<aui_dmx.h>
#include<aui_kl.h>
#include <ngl_types.h>
#include <ngl_log.h>
#include<ngl_dsc.h>

NGL_MODULE(NGLDSCR)
typedef struct{
   DWORD dmx;
   aui_hdl hdl;
   aui_hdl hkl;
   aui_attr_dsc attr;
   aui_attr_kl attrkl;
   ScrambleAlgo algo;
   USHORT pid;
   BOOL used;
   BOOL cwEncrypted;
   BYTE key[64];
   BYTE iv[32];
}NGLDSC;

#define NUM_DSCS 8
static NGLDSC nglDSCS[NUM_DSCS];
DWORD nglDscInit(){
    aui_attr_dsc attr;
    NGLOG_DEBUG("");
    aui_dsc_init(NULL,NULL);
    aui_kl_init(NULL,NULL);
    NGLOG_DEBUG("aui_dsc_init");
    aui_log_priority_set(AUI_MODULE_DSC,AUI_LOG_PRIO_DEBUG);
    aui_log_priority_set(AUI_MODULE_KL,AUI_LOG_PRIO_DEBUG);
    bzero(nglDSCS,sizeof(nglDSCS));
}

DWORD nglAllocDsc(DWORD dmx,INT pid){
    NGLDSC*dsc=NULL;
    aui_attr_dsc*attr;
    for(int i=0;i<NUM_DSCS;i++){
        if(!nglDSCS[i].used){
            dsc=nglDSCS+i;        break;
        }
    }
    if(NULL==dsc)return dsc;
    dsc->dmx=dmx;
    dsc->used=1;
    attr=&dsc->attr;
    bzero(attr,sizeof(aui_attr_dsc));
    attr->uc_dev_idx=dsc-nglDSCS;
    attr->dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
    attr->en_en_de_crypt = AUI_DSC_DECRYPT;
    aui_dsc_open(attr,&dsc->hdl);
    return (DWORD)dsc;
}


DWORD nglDscOpen(USHORT*pids,UINT cnt){
}
DWORD nglDscSetParameters(DWORD dwDescrambleID,const NGLDSC_Param*param){
    NGLDSC*dsc=(NGLDSC*)dwDescrambleID;
    aui_attr_dsc *attr=&dsc->attr;
    dsc->algo=param->algo;
    switch(param->algo){
    case eDSC_ALGO_DVB_CSA:
         attr->dsc_data_type = AUI_DSC_DATA_TS;
         attr->uc_algo = AUI_DSC_ALGO_CSA;
         attr->uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
         attr->puc_iv_ctr=NULL;
         break;
    case eDSC_ALGO_AES_128_CBC:
         attr->dsc_data_type = AUI_DSC_DATA_TS;
         attr->uc_algo = AUI_DSC_ALGO_AES;
         attr->uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
         if(NULL==param->pIV||0==param->uiIVLength)return NGL_INVALID_PARA;
             attr->puc_iv_ctr=dsc->iv;
         memcpy(dsc->iv,param->pIV,param->uiIVLength);
         break;
    }
}
static int InitHDL(NGLDSC*dsc){
    if(dsc->hdl==NULL){
        dsc->attr.dsc_key_type=(dsc->cwEncrypted?AUI_DSC_HOST_KEY_SRAM:AUI_DSC_CONTENT_KEY_KL);
        aui_dsc_open(&dsc->attr,dsc->hdl);
    }
    if(dsc->cwEncrypted&&dsc->hkl==NULL){
        aui_kl_open(&dsc->attrkl,dsc->hkl);
    }
}
DWORD nglDscClose(DWORD dwStbDescrHandle){
    NGLDSC*dsc=(NGLDSC*)dwStbDescrHandle;
    if(dsc->hdl){
        aui_dsc_close(dsc->hdl);
        dsc->hdl=NULL;
        dsc->pid=0;
        dsc->used=FALSE;
    }
}
DWORD nglGetCipherMode(NGLCipherMode*md){
}
DWORD nglSetCipherMode(NGLCipherMode md){
}
DWORD nglSetCipherSessionKey(const BYTE*key,UINT keylen ){
}

#define CHIPSET_OTP_ADDR (0)
DWORD nglGetChipID(){
    DWORD chipid;
    int rc=aui_otp_read(CHIPSET_OTP_ADDR,&chipid,sizeof(DWORD));//LSB
    NGLOG_VERBOSE("aui_otp_read=%d chipid=%08X\r\n",rc,chipid);
    return (rc==AUI_RTN_SUCCESS)?chipid:-1;
}
DWORD nglDscSetKeys(DWORD dwStbDescrHandle,const BYTE  *pOddKey,UINT32 uiOddKeyLength,
                const BYTE  *pEvenKey,UINT32 uiEvenKeyLength){
    NGLDSC*dsc=(NGLDSC*)dwStbDescrHandle;
    aui_attr_dsc *attr=&dsc->attr; 
    INT ret;
    InitHDL(dsc);
    if(dsc->cwEncrypted){
        UINT keysize,keypos;
        aui_kl_get(dsc->hkl,AUI_KL_GET_KEY_SIZE,&keysize);
        aui_kl_get(dsc->hkl,AUI_KL_GET_KEY_POS,&keypos);
    }else{
        attr->dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
        attr->puc_key = dsc->key;
        attr->ul_key_len = 0;//(pDesPara->nKeyLen == 0)?(64):(pDesPara->nKeyLen*8);     
    }
    attr->ul_pid_cnt = 1;
    attr->pus_pids = &dsc->pid;
    attr->ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
    attr->en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
    attr->en_residue = AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC;
    ret = aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc->attr);
    NGLOG_DEBUG("attach_key_info2dsc=%d hdl=%d pid=%d",ret,dsc->hdl,dsc->pid);
}
