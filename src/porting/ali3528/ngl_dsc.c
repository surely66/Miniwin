#include<ngl_dsc.h>
#include<aui_dsc.h>
#include<aui_dmx.h>
#include<aui_kl.h>
#include <ngl_types.h>
#include <ngl_log.h>

NGL_MODULE(DSCR)

#define INVALID_PID 0x1FFFF
typedef struct{
   aui_hdl hdl;
   aui_hdl hdl_kl;
   aui_attr_dsc attr;
   aui_cfg_kl klcfg;
   BYTE key[64];
   BYTE iv[32];
   BYTE pk[32];
   int algo;
   int schip_flag;
   UINT16 pid;
   NGLCipherMode cipherMode;
   int key_len;
}NGLDSC;

NGLSCHIP_ContentKey*pContentKey=NULL;
#define NUM_DSCS 8
static NGLDSC nglDSCS[NUM_DSCS];
static NGLCipherMode sCipherMode=eCM_INACTIVE;
static BYTE sSessionKey[256];//used to decrypt cw
static NGLDSC*GetNGDSC(UINT16 pid){
  int i;
  for(i=0;i<sizeof(nglDSCS)/sizeof(NGLDSC);i++)
    if(nglDSCS[i].pid==pid)
       return nglDSCS+i;
  return NULL;
}
#define CHECK(p) {if(p<nglDSCS||p>=&nglDSCS[NUM_DSCS])return NGL_INVALID_PARA;}
DWORD nglDscInit(){
   NGLOG_DEBUG("");
   aui_dsc_init(NULL,NULL);
   aui_kl_init(NULL,NULL);
   NGLOG_DEBUG("aui_dsc_init");
   aui_log_priority_set(AUI_MODULE_DSC,AUI_LOG_PRIO_DEBUG);
   aui_log_priority_set(AUI_MODULE_KL,AUI_LOG_PRIO_DEBUG);
   bzero(nglDSCS,sizeof(nglDSCS));
   
}

DWORD nglDscOpen(UINT16 pid)
{
    int i;
    NGLDSC*dsc=GetNGDSC(pid);
    NGLOG_DEBUG("");
    if(NULL!=dsc||pid>=0x1FFF){
        NGLOG_ERROR("pid %d exists or invalid pid",pid);
        return  NULL;
    }
    for(i=0;i<sizeof(nglDSCS)/sizeof(NGLDSC);i++){
        if(nglDSCS[i].pid==0){
           dsc=nglDSCS+i;
           break;
        }
    }
    bzero(&dsc->attr,sizeof(dsc->attr));
    bzero(&dsc->klcfg,sizeof(aui_cfg_kl));
    dsc->attr.uc_dev_idx = (dsc-nglDSCS);
    dsc->attr.dsc_data_type =AUI_DSC_DATA_TS;
    dsc->attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC;//AUI_DSC_RESIDUE_BLOCK_IS_RESERVED;
    dsc->attr.en_en_de_crypt=AUI_DSC_DECRYPT;//AUI_DSC_ENCRYPT;CSA must be DECRYPT(othrewise aui_dsc_attach_key_info2dsc will return error)

    dsc->attr.uc_algo = AUI_DSC_ALGO_CSA;
    dsc->attr.csa_version=AUI_DSC_CSA2;
    dsc->attr.uc_mode=AUI_DSC_WORK_MODE_IS_CBC;
    dsc->attr.puc_iv_ctr=NULL;

    dsc->hdl=NULL;
    dsc->pid=pid;
    dsc->schip_flag=0;
    dsc->attr.pus_pids=&dsc->pid;
    dsc->attr.ul_pid_cnt=1;
    NGLOG_DEBUG("\t %s dsc=%p pid=%d",__FUNCTION__,dsc,pid);
    return (DWORD)dsc;
}

DWORD nglDscClose(DWORD dwDescrambleID )
{
    NGLDSC*dsc=(NGLDSC*)dwDescrambleID;
    CHECK(dsc);
    NGLOG_DEBUG("%s dsc=%p hdl=%p hdl_kl=%p pid=%d",__FUNCTION__,dsc,dsc->hdl,dsc->hdl_kl,(dsc?dsc->pid:0));
    if(dsc->hdl!=NULL||0!=dsc->pid){
         dsc->pid=0;
         aui_dsc_close(dsc->hdl);
         aui_kl_close(dsc->hdl_kl);
         dsc->hdl_kl=NULL;
         dsc->hdl=NULL;
    }
    return NGL_OK;
}
static char*AuiAlgo(int algo)
{
   const char *names[]={"DES","AES","SHA","TDES","CSA"};
   if(algo<sizeof(names)/sizeof(char*)&&algo>=0)
      return names[algo];
   return "??";
}
static char*AuiKeyType(int tp){
   const char*names[]={"REG","SRAM","KL","OTP"};
   if(tp<sizeof(names)/sizeof(char*)&&tp>=0)
      return names[tp];
   return "??";
}
static INT OpenHDL(NGLDSC*dsc){
    int rc=0;
    if(NULL==dsc->hdl){
        aui_hdl hdl_dmx;
        aui_dmx_data_path dmx_path;
        dsc->attr.dsc_key_type =(0==dsc->schip_flag||AUI_DSC_ALGO_CSA==dsc->attr.uc_algo)?AUI_DSC_HOST_KEY_SRAM:AUI_DSC_CONTENT_KEY_KL;
        switch(dsc->attr.uc_algo){
        case AUI_DSC_ALGO_CSA:dsc->attr.en_en_de_crypt=AUI_DSC_ENCRYPT;break;//CSA must be DECRYPT
        case AUI_DSC_ALGO_AES:dsc->attr.en_en_de_crypt=AUI_DSC_ENCRYPT;break;
        }
        if((NULL==dsc->hdl)&&(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DSC,dsc->attr.uc_dev_idx ,&dsc->hdl)))
            aui_dsc_open(&dsc->attr,&dsc->hdl);
        
        if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX,0,&hdl_dmx))
            NGLOG_DEBUG("OpenHDL find_dev of DMX failed");

        bzero(&dmx_path,sizeof(dmx_path));
        dmx_path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
        dmx_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
        dmx_path.p_hdl_de_dev =dsc->hdl;
        rc=aui_dmx_data_path_set(hdl_dmx,&dmx_path);
        NGLOG_DEBUG_IF(rc,"OpenHDL aui_dmx_data_path_set=%d hdl_dmx=%p  dschdl=%p algo=%s",rc,hdl_dmx,dsc->hdl,AuiAlgo(dsc->attr.uc_algo));
        if(aui_find_dev_by_idx(AUI_MODULE_KL,dsc->attr.uc_dev_idx,&dsc->hdl_kl)){
            struct aui_attr_kl attr;
            memset(&attr,0,sizeof(aui_attr_kl));
            attr.uc_dev_idx = dsc->attr.uc_dev_idx;
            attr.en_key_pattern = (8==dsc->key_len) ? AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN:AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
            attr.en_level = AUI_KL_KEY_TWO_LEVEL;//AUI_KL_KEY_THREE_LEVEL AUI_KL_KEY_TWO_LEVEL AUI_KL_KEY_ONE_LEVEL
            attr.en_root_key_idx = AUI_KL_ROOT_KEY_0_0;
            attr.en_key_ladder_type=AUI_KL_TYPE_ALI;
            rc=aui_kl_open(&attr,&dsc->hdl_kl);
            NGLOG_DEBUG("dsc=%p aui_kl_open=%d hdl_kl=%p uc_dev_idx=%d",dsc,rc,dsc->hdl_kl,attr.uc_dev_idx);
        }
        NGLOG_DEBUG("dsc=%p dsc->hdl_kl=%p algo=%s",dsc,dsc->hdl_kl,AuiAlgo(dsc->attr.uc_algo));
    }
    switch(dsc->attr.dsc_key_type){
    case AUI_DSC_CONTENT_KEY_KL:{
            UINT sessionKeyLen=16;
            dsc->attr.puc_key=NULL;
            dsc->klcfg.run_level_mode =AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
            dsc->klcfg.en_kl_algo = AUI_KL_ALGO_TDES;//AUI_KL_ALGO_TDES; AUI_KL_ALGO_AES
            dsc->klcfg.en_crypt_mode = AUI_KL_DECRYPT;
            dsc->klcfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;
            memcpy(dsc->klcfg.ac_key_val,dsc->pk,16);///*sSessionKey*/,sizeof(sSessionKey));
            switch(dsc->attr.ul_key_pattern){
            case AUI_DSC_KEY_PATTERN_ODD_EVEN:dsc->klcfg.en_cw_key_attr=AUI_KL_CW_KEY_ODD_EVEN;break;
            case AUI_DSC_KEY_PATTERN_EVEN:dsc->klcfg.en_cw_key_attr=AUI_KL_CW_KEY_EVEN;break;
            case AUI_DSC_KEY_PATTERN_ODD:dsc->klcfg.en_cw_key_attr=AUI_KL_CW_KEY_ODD;break;
            }
            memcpy(dsc->klcfg.ac_key_val+sessionKeyLen,dsc->key,sizeof(dsc->key));//uiOddKeyLength+uiEvenKeyLength);
            rc=aui_kl_get(dsc->hdl_kl,AUI_KL_GET_KEY_POS,(void *)&dsc->attr.ul_key_pos);
            NGLOG_VERBOSE("aui_kl_get=%d ul_key_pos=%d",rc,dsc->attr.ul_key_pos);
            rc=aui_kl_get(dsc->hdl_kl,AUI_KL_GET_KEY_SIZE,(void*)&dsc->attr.ul_key_len);
            NGLOG_VERBOSE("aui_kl_get=%d ul_key_len=%d",rc,dsc->attr.ul_key_len);
            rc=aui_kl_gen_key_by_cfg(dsc->hdl_kl,&dsc->klcfg,&dsc->attr.ul_key_pos);
            NGLOG_DEBUG("aui_kl_gen_key_by_cfg=%d  hdl_kl=%p  ul_key_pos=%d",rc,dsc->hdl_kl,dsc->attr.ul_key_pos);
        }break;
    case AUI_DSC_HOST_KEY_SRAM:
        dsc->attr.puc_key=dsc->key;
        dsc->attr.uc_dev_idx = (dsc-nglDSCS);
        dsc->attr.en_en_de_crypt=AUI_DSC_DECRYPT;//AUI_DSC_ENCRYPT;CSA must be DECRYPT(othrewise aui_dsc_attach_key_info2dsc will return error)

        dsc->attr.uc_mode=AUI_DSC_WORK_MODE_IS_CBC;
        break;
    case AUI_DSC_HOST_KEY_REG:
    case AUI_DSC_CONTENT_KEY_OTP:
        break;
   }
   NGLOG_DEBUG("dsc %p dsc.hdl=%p dsc.hdl_kl=%p dsc.attr.uc_algo=%s dsc_key_type=%s",dsc,dsc->hdl,dsc->hdl_kl,
            AuiAlgo(dsc->attr.uc_algo),AuiKeyType(dsc->attr.dsc_key_type));
}

static const char*PRINTALGO(int a){
  switch(a){
  case eDSC_ALGO_DVB_CSA:return "DSC_ALGO_DVB_CSA";
  case eDSC_ALGO_AES_128_CBC:return "DSC_ALGO_AES_128_CBC";
  case eDSC_ALGO_DVB_CSA3_STANDARD_MODE:return "DSC_ALGO_DVB_CSA3_STANDARD_MODE";
  case eDSC_ALGO_DVB_CSA3_MINIMALLY_ENHANCED_MODE:return "DSC_ALGO_DVB_CSA3_MINIMALLY_ENHANCED_MODE";
  case eDSC_ALGO_DVB_CSA3_FULLY_ENHANCED_MODE:return "DSC_ALGO_DVB_CSA3_FULLY_ENHANCED_MODE";
  default:return "Unknown Algo";
  }
}

DWORD nglDscSetParameters(DWORD dwStbStreamHandle,const NGLDSC_Param *param )
{
    NGLDSC*dsc=(NGLDSC*)dwStbStreamHandle;
    CHECK(dsc);
    NGLOG_DEBUG("dsc=%p  algo=%s ivLength=%d",dsc,PRINTALGO(param->algo),param->uiIVLength);
    switch(param->algo){
    case eDSC_ALGO_DVB_CSA:
           dsc->attr.uc_algo = AUI_DSC_ALGO_CSA;
           dsc->attr.csa_version=AUI_DSC_CSA2;
           dsc->attr.uc_mode=AUI_DSC_WORK_MODE_IS_CBC;
           if(NULL!=param->pIV||param->uiIVLength)return NGL_INVALID_PARA;
           dsc->attr.puc_iv_ctr=NULL;
           break;
    case eDSC_ALGO_AES_128_CBC:
           dsc->attr.uc_algo = AUI_DSC_ALGO_AES;
           dsc->attr.uc_mode=AUI_DSC_WORK_MODE_IS_CBC;//uv_mode only use for AES/DES/TDES
           if(NULL==param->pIV||0==param->uiIVLength)return NGL_INVALID_PARA;
           dsc->attr.puc_iv_ctr=dsc->iv;

           memcpy(dsc->iv,param->pIV,param->uiIVLength);
           break;
    case eDSC_ALGO_DVB_CSA3_STANDARD_MODE:           dsc->attr.uc_algo = AUI_DSC_ALGO_DES;break;
    case eDSC_ALGO_DVB_CSA3_MINIMALLY_ENHANCED_MODE: dsc->attr.uc_algo = AUI_DSC_ALGO_TDES;break;
    case eDSC_ALGO_DVB_CSA3_FULLY_ENHANCED_MODE:     dsc->attr.uc_algo = AUI_DSC_ALGO_AES;break;
    default:break;
    }
	if(dsc->algo!=param->algo){
		aui_dsc_close(dsc->hdl);
		dsc->hdl=NULL;
		NGLOG_DEBUG("algo changed from %s->%s",PRINTALGO(dsc->algo),PRINTALGO(param->algo));
		dsc->algo=param->algo;
	}
    OpenHDL(dsc);
    return NGL_OK;
}
#if 1 
#define MAX_KEY_SIZE 16
static INT VA_DSCR_SCHIP_SetHostKeys(DWORD dwStbDescrHandle,
	UINT16 uiOddKeyLength, const BYTE  *pOddKey,
	UINT16 uiEvenKeyLength, const BYTE  *pEvenKey )
{
	aui_attr_dsc dsc_attr;
	unsigned char key_buffer[MAX_KEY_SIZE * 2] = {0};
        NGLDSC*dsc=(NGLDSC*)dwStbDescrHandle;	
	
	memcpy(key_buffer, pOddKey, uiOddKeyLength);	
	memcpy(key_buffer + uiOddKeyLength, pEvenKey, uiEvenKeyLength);	

	dsc_attr.puc_key = key_buffer;
	dsc_attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC;
	if (dsc->algo == eDSC_ALGO_AES_128_CBC) {
		dsc_attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
		dsc_attr.puc_iv_ctr = dsc->iv;
	}else{
		dsc_attr.csa_version = AUI_DSC_CSA2;
		if(uiEvenKeyLength > MAX_KEY_SIZE/2 || uiOddKeyLength > MAX_KEY_SIZE/2)
			return NGL_INVALID_PARA;
	}
	
	if(uiEvenKeyLength)
		dsc_attr.ul_key_len = uiEvenKeyLength * 8;
	if(uiOddKeyLength)
		dsc_attr.ul_key_len = uiOddKeyLength * 8;

	dsc_attr.en_en_de_crypt = AUI_DSC_DECRYPT;
	dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN; /*Odd & Even Keys are provided*/
	dsc_attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
	dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;	/*The parity is detected from TS packet header*/

	dsc_attr.ul_pid_cnt = 1;
	dsc_attr.pus_pids = (unsigned short *)(&(dsc->pid));


	if(dsc->hdl){
		if(aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc_attr)){
			NGLOG_DEBUG("va_dscr set key fail\n");
			goto err1;
		}
	}else{
		/*default using CSA algo, for TEST_CASE_3*/
		aui_hdl hdl;

		dsc_attr.uc_dev_idx = 0;
		dsc_attr.dsc_data_type = AUI_DSC_DATA_TS;
		dsc_attr.uc_algo = AUI_DSC_ALGO_CSA;
		dsc_attr.csa_version = AUI_DSC_CSA2;
		if(aui_dsc_open(&dsc_attr, &hdl)){
			NGLOG_ERROR("dsc open fail");
			return NGL_INVALID_PARA;
		}else{
			dsc->hdl = hdl;
			dsc->algo = eDSC_ALGO_DVB_CSA;
		}

		if(aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc_attr)){
			NGLOG_ERROR("va_dscr set key fail");
			goto err1;
		}
	}
	return NGL_OK;

err1:
	if(aui_dsc_close(dsc->hdl)){
		NGLOG_ERROR("dsc close fail");
		return NGL_ERROR;
	}
	return NGL_ERROR;
}

DWORD nglSchipSetKeys(DWORD dwStbDescrHandle,const BYTE  *pOddKey,UINT32 uiOddKeyLength,
		const BYTE  *pEvenKey,UINT32 uiEvenKeyLength){
	NGLDSC*dsc=(NGLDSC*)dwStbDescrHandle;
	aui_attr_dsc dsc_attr;
	aui_attr_kl kl_attr;
	aui_hdl kl_hdl;
	struct aui_cfg_kl cfg;
	unsigned char key_buffer[MAX_KEY_SIZE * 3] = {0};

	aui_cfg_kl cfg_kl;
	aui_kl_key_source_attr key_source;
	aui_kl_key_source_attr data_source;

	memcpy(key_buffer, dsc->pk, MAX_KEY_SIZE);
	memcpy(key_buffer + MAX_KEY_SIZE, pOddKey, uiOddKeyLength);
	memcpy(key_buffer + MAX_KEY_SIZE + uiOddKeyLength, pEvenKey, uiEvenKeyLength);
	NGLOG_DUMP("PK & CW:", key_buffer, MAX_KEY_SIZE + 2 * uiOddKeyLength);
	NGLOG_DEBUG("Setup 1 eChipsetMode: %d", pContentKey->eChipsetMode);

	if(pContentKey->eChipsetMode != dsc->cipherMode){
		NGLOG_DEBUG("%s (%d)\n", __FUNCTION__, __LINE__);
		if(VA_DSCR_SCHIP_SetHostKeys(dwStbDescrHandle, uiOddKeyLength, pOddKey, uiEvenKeyLength, pEvenKey)){
			NGLOG_ERROR("va_dscr_schip_sethostkeys error!\n");
			return NGL_ERROR;
		}
		return NGL_OK;
	}

	kl_attr.uc_dev_idx = (dsc-nglDSCS);
	kl_attr.en_level = AUI_KL_KEY_TWO_LEVEL;
	kl_attr.en_root_key_idx = AUI_KL_ROOT_KEY_0_0;	/*0x4d*/
	kl_attr.en_key_ladder_type = AUI_KL_TYPE_ALI;
	
	if(uiOddKeyLength == MAX_KEY_SIZE){
		NGLOG_DEBUG("FPA 128");
		kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
	}else{
		NGLOG_DEBUG("FPA 64");
		kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
	}

	if(!dsc->hdl_kl){
		if(aui_kl_open(&kl_attr, &kl_hdl)){
			NGLOG_ERROR("aui_kl_open fail");
			return NGL_ERROR;
		}
		dsc->hdl_kl = kl_hdl;
	}else{
		// Just need to keep generating key
		kl_hdl = dsc->hdl_kl;
		unsigned long key_dst_pos;
		cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
		cfg.en_kl_algo = AUI_KL_ALGO_TDES;
		cfg.en_crypt_mode = AUI_KL_DECRYPT;
		cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;	/*It is used for TS mode*/
		memcpy(cfg.ac_key_val, key_buffer, MAX_KEY_SIZE + 2 * uiOddKeyLength);
		if(aui_kl_gen_key_by_cfg(kl_hdl, &cfg, &key_dst_pos)){	/*just generate key*/
			NGLOG_ERROR("aui_kl_gen_key_by_cfg fail");
			goto err2;
		}
		NGLOG_DEBUG("1 key_dst_pos: %d", key_dst_pos);

		return NGL_OK;
	}

	unsigned long key_dst_pos;
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_TDES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;	/*It is used for TS mode*/
	memcpy(cfg.ac_key_val, key_buffer, MAX_KEY_SIZE + 2 * uiOddKeyLength);
	if(aui_kl_gen_key_by_cfg(kl_hdl, &cfg, &key_dst_pos)){
		NGLOG_ERROR("aui_kl_gen_key_by_cfg fail");
		goto err2;
	}

	NGLOG_DEBUG("2 key_dst_pos: %d", key_dst_pos);

	// set KL position to dsc
	dsc_attr.ul_key_pos = key_dst_pos;
	NGLOG_DEBUG("3 key_dst_pos: %d\n", key_dst_pos);
	dsc_attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC;

	if (dsc->algo == eDSC_ALGO_AES_128_CBC){//eSCRAMBLING_ALGO_AES_128_CBC) {
		NGLOG_DEBUG("eSCRAMBLING_ALGO_AES_128_CBC");
		dsc_attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
		dsc_attr.puc_iv_ctr = dsc->iv;
	} else{
		NGLOG_DEBUG("eSCRAMBLING_ALGO_DVB_CSA");
		dsc_attr.csa_version = AUI_DSC_CSA2;
		if(uiEvenKeyLength > MAX_KEY_SIZE/2 || uiOddKeyLength > MAX_KEY_SIZE/2)
			return NGL_INVALID_PARA;
	}

	dsc_attr.ul_key_len = uiEvenKeyLength * 8;
	dsc_attr.en_en_de_crypt = AUI_DSC_DECRYPT;
	dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN; /*Odd & Even Keys are provided*/
	dsc_attr.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;	/*The parity is detected from TS packet header*/
	dsc_attr.ul_pid_cnt = 1;
	dsc_attr.pus_pids = (unsigned short *)(&(dsc->pid));

	if(dsc->hdl) {
		if(aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc_attr)){
			NGLOG_DEBUG("va_dscr set key fail");
			goto err1;
		}
	}else{
		/*default using CSA algo, for TEST_CASE_3*/
		aui_hdl hdl;
		dsc_attr.uc_dev_idx = 0;
		dsc_attr.dsc_data_type = AUI_DSC_DATA_TS;
		dsc_attr.uc_algo = AUI_DSC_ALGO_CSA;
		dsc_attr.csa_version = AUI_DSC_CSA2;
		if(aui_dsc_open(&dsc_attr, &hdl)){
			NGLOG_ERROR("dsc open fail");
			return NGL_INVALID_PARA;
		}else{
			dsc->hdl = hdl;
			dsc->algo =eDSC_ALGO_DVB_CSA;// eSCRAMBLING_ALGO_DVB_CSA;
		}

		if(aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc_attr)){
			NGLOG_ERROR("va_dscr set key fail");
			goto err1;
		}
	}
	return NGL_OK;
err2:
	if(aui_kl_close(kl_hdl)){
		NGLOG_ERROR("kl close fail");
		return NGL_ERROR;
	}
err1:
	if(aui_dsc_close(dsc->hdl)){
		NGLOG_ERROR("dsc close fail");
		return NGL_ERROR;
	}
	return NGL_ERROR; 
	return NGL_OK;
}
#endif

DWORD nglDscSetKeys(DWORD dwStbDescrHandle,const BYTE  *pOddKey,UINT32 uiOddKeyLength,
         const BYTE  *pEvenKey,UINT32 uiEvenKeyLength)
{
    int ret;
    BYTE*pk;
    NGLDSC*dsc=(NGLDSC*)dwStbDescrHandle;
    //NGLOG_DUMP("ODD",pOddKey,uiOddKeyLength);
    //NGLOG_DUMP("EVEN",pEvenKey,uiEvenKeyLength);
    CHECK(dsc);
    NGLOG_DEBUG("dsc=%p dsc->hdl=%p OddKey=%p/%d EvenKey=%p/%d schip_flag=%d",dsc,dsc->hdl,pOddKey,uiOddKeyLength,pEvenKey,uiEvenKeyLength,dsc->schip_flag);
    
	if(dsc->schip_flag){
		return nglSchipSetKeys(dwStbDescrHandle,pOddKey,uiOddKeyLength,pEvenKey,uiEvenKeyLength);
	}
    if(NULL==pOddKey)uiOddKeyLength=0;
    if(NULL==pEvenKey)uiEvenKeyLength=0;

    if ( (0==uiOddKeyLength+uiEvenKeyLength)||(uiOddKeyLength+uiEvenKeyLength)%8 )
        return NGL_INVALID_PARA;

    if(8==uiOddKeyLength||8==uiEvenKeyLength) dsc->attr.uc_algo=AUI_DSC_ALGO_CSA;
    if(16==uiOddKeyLength||16==uiEvenKeyLength) dsc->attr.uc_algo=AUI_DSC_ALGO_AES;
    
    switch(dsc->attr.uc_algo){
    case AUI_DSC_ALGO_CSA:dsc->attr.ul_key_len=8*8;
            if(uiOddKeyLength>8||uiEvenKeyLength>8){ NGLOG_ERROR("CSA.NGL_INVALID_PARA");return NGL_INVALID_PARA;}
            dsc->attr.puc_iv_ctr=NULL;
            if(pOddKey){
                pk=(BYTE*)pOddKey;
                pk[3]=pk[0]+pk[1]+pk[2];
                pk[7]=pk[4]+pk[5]+pk[6];
            }
            if(pEvenKey){
                pk=(BYTE*)pEvenKey;
                pk[3]=pk[0]+pk[1]+pk[2];
                pk[7]=pk[4]+pk[5]+pk[6];
            }
            break;
    case AUI_DSC_ALGO_AES:dsc->attr.ul_key_len=16*8;
            if(uiOddKeyLength>16||uiEvenKeyLength>16){NGLOG_ERROR("AES.NGL_INVALID_PARA");return NGL_INVALID_PARA;}
            dsc->attr.puc_iv_ctr=dsc->iv;
            break;
    }
    if(NULL!=pOddKey && NULL!=pEvenKey)
         dsc->attr.ul_key_pattern= AUI_DSC_KEY_PATTERN_ODD_EVEN;
    else if(NULL==pOddKey)
         dsc->attr.ul_key_pattern= AUI_DSC_KEY_PATTERN_EVEN;
    else
         dsc->attr.ul_key_pattern= AUI_DSC_KEY_PATTERN_ODD;

    if(pOddKey&&uiOddKeyLength)
        memcpy(dsc->key,   pOddKey ,uiOddKeyLength);

    if(pEvenKey&&uiEvenKeyLength)
        memcpy(dsc->key+uiOddKeyLength , pEvenKey , uiEvenKeyLength );
    //NGLOG_DUMP("ODDEVEN",dsc->key,uiOddKeyLength+uiEvenKeyLength);
    dsc->attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;

    OpenHDL(dsc);
    ret=aui_dsc_attach_key_info2dsc(dsc->hdl,&dsc->attr);
    NGLOG_DEBUG("attach_key_info2dsc=%d dsc=%p KeyLen=%d/%d hdl=%p iv=%p algo=%s",ret,dsc,uiOddKeyLength,uiEvenKeyLength,
           dsc->hdl,dsc->attr.puc_iv_ctr,AuiAlgo(dsc->attr.uc_algo));
    return ret==0?NGL_OK:NGL_ERROR;
}

#define CHIPSET_OTP_ADDR (0)
#define KEY_OTP_ADDR	(0x03 * 4)
#define KL_KEY_OTP_SET	(1 << 23)

DWORD nglGetCipherMode(NGLCipherMode*md){
    DWORD data;
    AUI_RTN_CODE ret;
    ret= aui_otp_read(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
    *md=sCipherMode;
    if( (data & KL_KEY_OTP_SET) && (sCipherMode==eCM_LOCKED) )
       *md=eCM_LOCKED;
    else *md=sCipherMode;
    return NGL_OK;
}

DWORD nglSetCipherMode(NGLCipherMode md){
    DWORD data;
    AUI_RTN_CODE ret;
    switch(md){//
    case eCM_INACTIVE:
          ret = aui_otp_read(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
          if( (AUI_RTN_SUCCESS!=ret) || ((data&KL_KEY_OTP_SET)==1) )
             return NGL_ERROR;
          sCipherMode=eCM_INACTIVE;
          break;
    case eCM_SESSION:
          ret = aui_otp_read(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
          if( (AUI_RTN_SUCCESS!=ret) || ((data&KL_KEY_OTP_SET)==1) )
             return NGL_ERROR;
          sCipherMode=eCM_SESSION;
          break;
    case eCM_LOCKED:
          ret = aui_otp_read(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
          if(AUI_RTN_SUCCESS!=ret)return NGL_ERROR;
          data|=KL_KEY_OTP_SET;
          // because once OTP has been written, this chip can't be reversible. 
          //ret = aui_otp_write(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
          sCipherMode=eCM_LOCKED;
          break;
    default:return NGL_INVALID_PARA;
    }
    return NGL_OK;
}

DWORD nglSetCipherSessionKey(const BYTE*pSessionKey,UINT uiSessionKeyLength){
    memcpy(sSessionKey,pSessionKey,uiSessionKeyLength);
    DWORD index, index2;
    NGLDSC *pStbSession,*pStbSession2;
    //NGLDSC*dsc;	
    if(!pSessionKey || uiSessionKeyLength == 0 ||(uiSessionKeyLength % 8)){
	return NGL_INVALID_PARA;
    }
	
    for(index = 0; index < NUM_DSCS; index++ ) {
 	if(nglDSCS[index].pid) {
	    pStbSession =&nglDSCS[index];// (tVA_DSCR_StbSession *)vaDscrHandle[index];
	    for(index2 = 0; index2 < NUM_DSCS; index2++){
		if(index2 == index)
			continue;
		if(nglDSCS[index2].pid){
 		    pStbSession2 =&nglDSCS[index2];// (tVA_DSCR_StbSession *)vaDscrHandle[index2];
		    break;
		}

 		if(index2 == NUM_DSCS - 1)
                    NGLOG_ERROR("");return NGL_ERROR;
	    }
	    break;
        }

        if(index == NUM_DSCS - 1)
  	   return NGL_OK;	/*For SCHIP_CASE_2*/
    }

    pStbSession->schip_flag = 1;
    pStbSession2->schip_flag = 1;
    pStbSession->cipherMode = sCipherMode;//pContentKey->eChipsetMode;
    pStbSession2->cipherMode = sCipherMode;//pContentKey->eChipsetMode;*/
    memcpy(pStbSession->pk, pSessionKey, uiSessionKeyLength);
    memcpy(pStbSession2->pk, pSessionKey, uiSessionKeyLength);
    NGLOG_DEBUG("=index=%d index2=%d sCipherMode=%d",index,index2,sCipherMode);
    NGLOG_DUMP("SessionKey",pSessionKey,uiSessionKeyLength);
    return NGL_OK;
}

#define CHIPSET_OTP_ADDR (0)
DWORD nglGetChipID(){
     DWORD chipid;
     BYTE ids[8];
     int rc=aui_otp_read(CHIPSET_OTP_ADDR,&chipid,sizeof(DWORD));//LSB
     NGLOG_DEBUG("aui_otp_read=%d chipid=%08X\r\n",rc,chipid);
     return (rc==AUI_RTN_SUCCESS)?chipid:-1;//0x002013BA;//chipid;
}

