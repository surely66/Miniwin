/*
  FILE : stub_pvr.c
  PURPOSE: This file is a stub for linking tests.
*/
#include <stdio.h>
extern "C"{
#include <va_pvr.h>
}
#include <ngl_types.h>
#include <ngl_log.h>
#include <ngl_pvr.h>
#include <va_os.h>
#include <aui_common.h>
#include <dvbepg.h>
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

static int ETYPE2AUI(int tp){
   switch(tp){
   case 1:
   case 2:return AUI_DECV_FORMAT_MPEG;
   case 3:return AUI_DECA_STREAM_TYPE_MPEG1;
   case 4:return AUI_DECA_STREAM_TYPE_MPEG2;
   }
}

INT GetPvrParamByEcmPID(NGLPVR_RECORD_PARAM*param,WORD ecmpid){
    int i,audidx,rc;
    WORD pcr;
    BYTE pmtbuffer[1024];
    SERVICELOCATOR cur;    
    ELEMENTSTREAM es[16];
    DtvGetCurrentService(&cur);
    DtvGetServicePmt(&cur,pmtbuffer);
    PMT pmt(pmtbuffer,false);
    rc=pmt.getElements(es,false);
    memset(param,0,sizeof(NGLPVR_RECORD_PARAM));
    param->pcr_pid=pmt.pcrPid();
    for(i=0,audidx=0;i<rc;i++){
        switch(es[i].stream_type){
        case 1:
        case 2: param->video_pid=es[i].pid;
                param->video_type=0;//ETYPE2AUI(es[i].type);
                break;
        case 3:
        case 4: param->audio_pids[audidx]=es[i].pid;
                param->audio_types[audidx++]=ETYPE2AUI(es[i].stream_type);
                break;
        }
    }
    return (rc>0)?NGL_OK:NGL_ERROR;
}

DWORD VA_PVR_Start( DWORD  dwAcsId,int eRecordType,WORD sid){
    SERVICELOCATOR cur;
    BYTE pmtbuffer[1024];
    DtvGetCurrentService(&cur);
    DtvGetServicePmt(&cur,pmtbuffer);
    PMT pmt(pmtbuffer,false);
    WORD pid=pmt.ecmPid();
    NGLOG_DEBUG("service=%d.%d.%d ecmpid=%d",cur.netid,cur.tsid,cur.sid,pid);
    if(pid!=0x1FFF)
         return VA_PVR_OpenEcmChannel(dwAcsId,(tVA_PVR_RecordType)eRecordType,pid);
    return 0;
}

DWORD VA_PVR_OpenEcmChannel ( DWORD  dwAcsId,tVA_PVR_RecordType eRecordType,WORD wEcmChannelPid )
{//ecm is from PMT's CA_Descriptor
    int i,rc;
    static int indexed=0;
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
    sprintf(fname,"PVR_%d-%d-%d_%02d%02d%02d_%d",1900+tm.uiYear,tm.uiMonth,tm.uiMonthDay,tm.uiHour,tm.uiMin,tm.uiSec,indexed++);
    NGLPVR_RECORD_PARAM param;
    memset(&param,0,sizeof(NGLPVR_RECORD_PARAM));
    if(NGL_OK!=GetPvrParamByEcmPID(&param,wEcmChannelPid)){
         NGLOG_ERROR("invalid ECMPID 0x%x/%d",wEcmChannelPid,wEcmChannelPid);
         return kVA_ILLEGAL_HANDLE;
    }
    pvr->nglpvr=nglPvrRecordOpen(fname,&param);
    if(pvr->nglpvr!=0){
        param.recordMode=eRecordType;
        NGLOG_DEBUG("pvr=%p acsid=%d eRecordType=%d wEcmChannelPid=%d nglhandle=%p",pvr,dwAcsId,eRecordType,wEcmChannelPid,pvr->nglpvr);
        return (DWORD)pvr;
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

extern "C" void nglGetPvrPath(DWORD handler,char*path);

INT VA_PVR_RecordEcm (  DWORD        dwStbEcmChannelHandle,
                        UINT32       uiEcmLength,
                        BYTE *       pEcm,
                        void *       pUserData )
{
    PVR*pvr=(PVR*)dwStbEcmChannelHandle;
    char path[512];

    NGLOG_DEBUG("pvr=%p pEcm=%p uiEcmLength=%d",pvr,pEcm,uiEcmLength);
    
    if(pvr<sPvrs||pvr>=&sPvrs[MAX_PVR] ||NULL==pEcm||0==uiEcmLength||kVA_PVR_ECM_MAX_SIZE<uiEcmLength)
        return kVA_INVALID_PARAMETER;

    NGLOG_DUMP("==ECMData",pEcm,uiEcmLength/8);
    nglGetPvrPath(pvr->nglpvr,path);
    strcat(path,"/acs_ecm.dat");
    pvr->file=fopen(path,"wb");
    NGLOG_DEBUG("path=%s file=%p",path,pvr->file);
    fwrite(pEcm,1,uiEcmLength,pvr->file);
    fclose(pvr->file);
    return NGL_OK;
}

static BYTE PVR_MetaData[kVA_PVR_METADATA_MAX_SIZE];
INT VA_PVR_WriteMetadata (  DWORD    dwAcsId,
                            UINT32   uiMetadataLength,
                            BYTE *   pMetadata )
{//metadata<--->service ?
    char path[512];
    NGLOG_DEBUG("acsid=%d pMetadata=%p uiMetadataLength=%d",dwAcsId,pMetadata,uiMetadataLength);
    if(dwAcsId>= kVA_SETUP_NBMAX_ACS||kVA_PVR_METADATA_MAX_SIZE<uiMetadataLength||0==uiMetadataLength|| NULL==pMetadata)
        return kVA_INVALID_PARAMETER;
    sprintf(path,"/tmp/meta%d.dat",dwAcsId);
    FILE*f=fopen(path,"wb");
    if(f){
        fwrite(pMetadata,1,uiMetadataLength,f);
        fclose(f);
    }
    NGLOG_DEBUG("metapath=%s file=%p",path,f);
    memcpy(PVR_MetaData,pMetadata,uiMetadataLength);
    NGLOG_DUMP("MetaData",pMetadata,uiMetadataLength);
    return 0;
}


INT VA_PVR_ReadMetadata (   DWORD    dwAcsId,
                            UINT32   uiMetadataLength,
                            BYTE *   pMetadata )
{
    char path[512];
    NGLOG_DEBUG("acsid=%d pMetadata=%p uiMetadataLength=%d kVA_PVR_METADATA_MAX_SIZE=%d kVA_SETUP_NBMAX_ACS=%d",dwAcsId,pMetadata,
            uiMetadataLength,kVA_PVR_METADATA_MAX_SIZE,kVA_SETUP_NBMAX_ACS);
    if((dwAcsId>= kVA_SETUP_NBMAX_ACS)||(kVA_PVR_METADATA_MAX_SIZE<uiMetadataLength)||(0==uiMetadataLength|| NULL==pMetadata))
        return kVA_INVALID_PARAMETER;
    //fread(pMetadata,1,uiMetadataLength,pvr->file);
    sprintf(path,"/tmp/meta%d.dat",dwAcsId);
    FILE*f=fopen(path,"rb");
    if(f){
        fread(pMetadata,1,uiMetadataLength,f);
        fclose(f);
    }
    NGLOG_DEBUG("metapath=%s file=%p",path,f);
    memcpy(pMetadata,PVR_MetaData,uiMetadataLength);
    return 0;
}


#endif
/* End of File */
