/*
  FILE : stub_pvr.c
  PURPOSE: This file is a stub for linking tests.
*/
#include "va_pvr.h"
#include <stdio.h>
#include <ngl_types.h>
#include <ngl_log.h>
#include <ngl_pvr.h>
#include <va_os.h>

NGL_MODULE(ACSPVR)
#define TAG_CA_DESCRIPTOR 9

#if 1//defined (VAOPT_ENABLE_PVR)

//VAOPT_ENABLE_PVR not defined
typedef struct{
   tVA_PVR_RecordType recordType;//ePERSISTENT, eTIMESHIFT 
   WORD ecmPid;
   FILE*file;
   DWORD nglpvr;
}PVR;
#define MAX_PVR 8
static PVR sPvrs[MAX_PVR];

extern BYTE PMTSection[];
typedef struct{
    WORD pid;
    int type;
}Element;

//ecmpid used to find ecmpid int pmt ,if not found,mustreturn 0
static int getElements(BYTE*pmt,Element *pids,WORD ecmpid,WORD *pcr){
    int deslen=((pmt[10]&0x0F)<<8)|pmt[11];
    int dlen,slen= (pmt[1]&0x0F)<<8|pmt[2];
    int ecmpid_found=0;
    BYTE*pd=pmt+12+deslen;
    *pcr=(pmt[8]&0x1F)<<8|pmt[9];
    Element*pi=pids;
    {  BYTE*pi=pmt+12;
         int i,len=((pmt[10]&0x0F)<<8)|pmt[11];
         for(i=0;i<len;){
              if( (TAG_CA_DESCRIPTOR==pi[0]) ){
                   NGLOG_INFO("TAG:0x%x len=%d ecmpid=0x%x/%d",pi[i],pi[i+1],ecmpid,ecmpid);
                   NGLOG_DUMP("DescriptorData",&pi[i],pi[i+1]+2);
                   
                   if ( (pi[i+4]&0x01F)<<8|pi[i+5]==ecmpid){
                       ecmpid_found++;break;
                   }
              }
              i+=pi[1]+2;
         }
    }
    NGLOG_DEBUG("pcrpid=%d",*pcr);
    for(;pd<pmt+slen-1;pi++){
       pi->type=pd[0];
       pi->pid=(pd[1]&0x1F)<<8|pd[2];
       dlen=(pd[3]&0x0F)<<8|pd[4];
       pd+=dlen+5;
       NGLOG_INFO("\ttype=%d pid=%d",pi->type,pi->pid);
    }
    return ecmpid_found?pi-pids:0;
}

INT GetPvrParamByEcmPID(NGLPVR_RECORD_PARAM*param,WORD ecmpid){
    int i,audidx,rc;
    WORD pcr;
    Element es[16];
    rc=getElements(PMTSection,es,ecmpid,&pcr);
    memset(param,0,sizeof(NGLPVR_RECORD_PARAM));
    param->pcr_pid=pcr;
    for(i=0,audidx=0;i<rc;i++){
        switch(es[i].type){
        case 1:
        case 2: param->video_pid=es[i].pid;break;
        case 3:
        case 4: param->audio_pids[audidx++]=es[i].pid;
                break;
        }
    }
    return (rc>0)?NGL_OK:NGL_ERROR;
}

DWORD VA_PVR_OpenEcmChannel ( DWORD  dwAcsId,tVA_PVR_RecordType eRecordType,WORD wEcmChannelPid )
{//ecm is from PMT's CA_Descriptor
    int i,rc;
    char fname[256];
    PVR*pvr=NULL;
    if(dwAcsId>= kVA_SETUP_NBMAX_ACS || wEcmChannelPid>=0x1FFF)
        return kVA_ILLEGAL_HANDLE;
    if(eRecordType!=ePERSISTENT&&eRecordType!=eTIMESHIFT)
        return kVA_ILLEGAL_HANDLE;
    
    for(i=0;i<MAX_PVR;i++){
        if(sPvrs[i].ecmPid==0){
            pvr=sPvrs+i;
            pvr->ecmPid=wEcmChannelPid;
            pvr->recordType=eRecordType;
            break;
        }
    }
    tVA_OS_Time tnow;
    tVA_OS_Tm tm;
    VA_OS_GetTime(&tnow);
    VA_OS_TimeToTm(&tnow,&tm);
    sprintf(fname,"PVR_%d-%d-%d_%02d%02d%02d",tm.uiYear,tm.uiMonth,tm.uiMonthDay,tm.uiHour,tm.uiMin,tm.uiSec);
    NGLPVR_RECORD_PARAM param;
    if(NGL_OK!=GetPvrParamByEcmPID(&param,wEcmChannelPid)){
         NGLOG_ERROR("invalid ECMPID 0x%x/%d",wEcmChannelPid,wEcmChannelPid);
         return kVA_ILLEGAL_HANDLE;
    }
    pvr->nglpvr=nglPvrRecordOpen(fname,&param);
    if(pvr->nglpvr!=0){
        param.recordMode=eRecordType;
        NGLOG_DEBUG("pvr=%p acsid=%d eRecordType=%d wEcmChannelPid=%d nglhandle=%p",pvr,dwAcsId,eRecordType,wEcmChannelPid,pvr->nglpvr);
        return pvr;
    }
    return  kVA_ILLEGAL_HANDLE;
}


INT VA_PVR_CloseEcmChannel ( DWORD   dwStbEcmChannelHandle )
{
    PVR*pvr=(PVR*)dwStbEcmChannelHandle;
    if(pvr<sPvrs||pvr>=&sPvrs[MAX_PVR]){
        return kVA_INVALID_PARAMETER;
    }
    if(0==pvr->nglpvr){
        return kVA_INVALID_PARAMETER;
    }
    NGLOG_DEBUG(" pvr=%p nglhandle=%p",pvr,pvr->nglpvr);
    nglPvrRecordClose(pvr->nglpvr);
    pvr->nglpvr=0;
    return 0;
}

extern void nglGetPvrPath(DWORD handler,char*path);

INT VA_PVR_RecordEcm (  DWORD        dwStbEcmChannelHandle,
                        UINT32       uiEcmLength,
                        BYTE *       pEcm,
                        void *       pUserData )
{
    PVR*pvr=(PVR*)dwStbEcmChannelHandle;
    char path[512];

    NGLOG_DEBUG("pvr=%p pEcm=%p uiEcmLength=%d ,path=%s",pvr,pEcm,uiEcmLength);
    
    if(pvr<sPvrs||pvr>=&sPvrs[MAX_PVR] ||NULL==pEcm||0==uiEcmLength||kVA_PVR_ECM_MAX_SIZE<uiEcmLength)
        return kVA_INVALID_PARAMETER;

    NGLOG_DUMP("==ECMData",pEcm,uiEcmLength/8);
    nglGetPvrPath(pvr->nglpvr,path);
    strcat(path,"/acs_ecm.dat");
    pvr->file=fopen(path,"wb");
    fwrite(pEcm,1,uiEcmLength,pvr->file);
    fclose(pvr->file);
    return NGL_OK;
}


INT VA_PVR_WriteMetadata (  DWORD    dwAcsId,
                            UINT32   uiMetadataLength,
                            BYTE *   pMetadata )
{//metadata<--->service ?
    NGLOG_DEBUG(" pMetadata=%p uiMetadataLength=%d",pMetadata,uiMetadataLength);
    if(dwAcsId>= kVA_SETUP_NBMAX_ACS||kVA_PVR_METADATA_MAX_SIZE<uiMetadataLength||0==uiMetadataLength|| NULL==pMetadata)
        return kVA_INVALID_PARAMETER;
    //kVA_PVR_METADATA_MAX_SIZE=64
    //fwrite(pMetadata,1,uiMetadataLength,pvr->file);
    NGLOG_DUMP("MetaData",pMetadata,uiMetadataLength);
    return 0;
}


INT VA_PVR_ReadMetadata (   DWORD    dwAcsId,
                            UINT32   uiMetadataLength,
                            BYTE *   pMetadata )
{
    NGLOG_DEBUG("pMetadata=%p uiMetadataLength=%d",pMetadata,uiMetadataLength);
    if(dwAcsId>= kVA_SETUP_NBMAX_ACS||kVA_PVR_METADATA_MAX_SIZE<uiMetadataLength||0==uiMetadataLength|| NULL==pMetadata)
        return kVA_INVALID_PARAMETER;
    //fread(pMetadata,1,uiMetadataLength,pvr->file);
    return 0;
}


#endif
/* End of File */
