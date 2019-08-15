/*
  FILE : stub_xnvm.c
  PURPOSE: This file is a stub for linking tests.
*/

#include <stdio.h>
#if 1 //(!defined VAOPT_ENABLE_ACS30 && !defined VAOPT_ENABLE_ACS31)
#include "va_xnvm.h"
#include "aui_flash.h"
#include <va_setup.h>
#include <ngl_types.h>
#include <ngl_log.h>

NGL_MODULE(ACSXNVM)

typedef struct{
  UINT32 addr;
  UINT32 size;
  FILE*file;
  const char*fname;
  aui_hdl hdl;
}NVMSEGMENT;

#define XNVM_START_ADDR     0x4300000  //ALI3528
#define XNVM_PARTITION_SIZE 0x0080000  //ALI3528 

static NVMSEGMENT Segments[eSEGMENT_LAST]={
  //init segment addr and size eSEGMENT_SOFTWARE = 0, eSEGMENT_ACS_DATA_1, eSEGMENT_ACS_DATA_2, eSEGMENT_LOADER_DATA
  {XNVM_START_ADDR+kVA_SETUP_ACS_DATA_SEGMENT_SIZE*0,kVA_SETUP_ACS_DATA_SEGMENT_SIZE,NULL,"./software.dat",NULL},//this segment is optional
  {XNVM_START_ADDR+kVA_SETUP_ACS_DATA_SEGMENT_SIZE*1,kVA_SETUP_ACS_DATA_SEGMENT_SIZE,NULL,"./acsdata1.dat",NULL},
  {XNVM_START_ADDR+kVA_SETUP_ACS_DATA_SEGMENT_SIZE*2,kVA_SETUP_ACS_DATA_SEGMENT_SIZE,NULL,"./acsdata2.dat",NULL},
  {XNVM_START_ADDR+kVA_SETUP_ACS_DATA_SEGMENT_SIZE*3,kVA_SETUP_ACS_DATA_SEGMENT_SIZE,NULL,"./aceloader.dat",NULL}//this segment is optional
};

#define USE_FILE 1
DWORD   VA_XNVM_Open(DWORD dwVaXnvmHandle, tVA_XNVM_Segment eSegment)
{
    aui_flash_open_param open_param;
    aui_hdl flash_handle = 0;
    NVMSEGMENT*s=NULL;
    if(eSegment<0||eSegment>= eSEGMENT_LAST)
        return  kVA_ILLEGAL_HANDLE;
    s=Segments+eSegment;
    if(s->file!=NULL||s->hdl!=NULL){
       printf("VA_XNVM_Open file=%p ,hdl=%p\r\n",s->file,s->hdl);
       return  kVA_ILLEGAL_HANDLE;
    }
#ifndef USE_FILE
    static int inited=0;
    if(0==inited){
        aui_flash_init(NULL,NULL);
        inited++;
    }
    bzero(&open_param,sizeof(open_param));
    open_param.flash_id =eSegment;//?
    open_param.flash_type =AUI_FLASH_TYPE_NOR;//AUI_FLASH_TYPE_NOR;//AUI_FLASH_TYPE_NAND;
    printf("%s  dwVaXnvmHandle=0x%x  segment=%d\r\n",__FUNCTION__,dwVaXnvmHandle,eSegment);
    AUI_RTN_CODE err = aui_flash_open(&open_param, &flash_handle);
    aui_flash_info flash_info;
    bzero(&flash_info,sizeof(flash_info));
    printf("aui_flash_open segment %d result=%d  flash_handle=%p\r\n",eSegment,err,&flash_handle);
    if(err)
        return kVA_ILLEGAL_HANDLE;
    err=aui_flash_info_get(flash_handle, &flash_info);
    printf("VA_XNVM_Open.aui_flash_info_get=%d segment %d startaddr=%p size=0x%x blockcng=%d,blksize=0x%x\r\n",err,eSegment,flash_info.star_address,
           flash_info.flash_size, flash_info.block_cnt,flash_info.block_size);
    s->hdl=flash_handle;
    s->size=flash_info.flash_size;
#else
    s->file=fopen(s->fname,"ab+");
    printf("fname=%s file=%p  size=0x%x\r\n",s->fname,s->file,s->size);
    fseek(s->file,0,SEEK_END);
    long i,fsize=ftell(s->file);
    if(s->file&&(fsize<s->size)){
        int cnt=(s->size-fsize+127)/128;
        unsigned char buf[128];
        memset(buf,0xFF,128); 
        for(i=0;i<cnt;i++)
           fwrite(buf,128,1,s->file);
    }
#endif
    return (DWORD)s;
}

INT VA_XNVM_Close(DWORD dwStbXnvmHandle)
{
    NVMSEGMENT*s=(NVMSEGMENT*)dwStbXnvmHandle;
    printf("%s\r\n",__FUNCTION__);
    if(s<Segments||s>=&Segments[eSEGMENT_LAST])
        return kVA_INVALID_PARAMETER;
    if(s->hdl==NULL&&s->file==NULL)
        return kVA_ILLEGAL_HANDLE;
#ifndef USE_FILE
    if(s->hdl)
        aui_flash_close(s->hdl);
#else
    if(s->file);
        fclose(s->file);
#endif
    s->hdl=NULL;
    s->file=NULL;
    return kVA_OK;
}

INT VA_XNVM_Read (DWORD dwStbXnvmHandle, UINT32 uiOffset, UINT32 uiSize, BYTE* pReadData)
{
   NVMSEGMENT*s=(NVMSEGMENT*)dwStbXnvmHandle;
   INT32 ret,readed;
   printf("%s offset=0x%x size=0x%x\r\n",__FUNCTION__,uiOffset,uiSize);
   if(s<Segments||s>=&Segments[eSEGMENT_LAST])
       return kVA_INVALID_PARAMETER;
   if(uiOffset+uiSize>s->size||pReadData==NULL||uiSize ==0)
       return kVA_INVALID_PARAMETER;
  if(s->hdl==NULL&&s->file==NULL)
       return kVA_ILLEGAL_HANDLE;
#ifndef USE_FILE
   aui_flash_read(s->hdl,s->addr+uiOffset,uiSize,&readed,pReadData);
#else
   ret=fseek(s->file,uiOffset,SEEK_SET);
   readed=fread(pReadData,1,uiSize,s->file);
#endif
   printf("readed=0x%x return kVA_OK fseek=%d\r\n",readed,ret);
   VA_XNVM_RequestDone(dwStbXnvmHandle);
   return kVA_OK;
}

INT  VA_XNVM_Write(DWORD dwStbXnvmHandle, UINT32 uiOffset, UINT32 uiSize, BYTE* pWriteData)
{
  NVMSEGMENT*s=(NVMSEGMENT*)dwStbXnvmHandle;
  INT ret, writed;
  printf("%s offset=0x%x size=0x%x\r\n",__FUNCTION__,uiOffset,uiSize);
  if(s<Segments||s>=&Segments[eSEGMENT_LAST])
       return kVA_INVALID_PARAMETER;
  if(uiOffset+uiSize>s->size||pWriteData==NULL||uiSize ==0)
       return kVA_INVALID_PARAMETER;
  if(s->hdl==NULL&&s->file==NULL)
       return kVA_ILLEGAL_HANDLE;
#ifndef USE_FILE
  aui_flash_write(s->hdl,s->addr+uiOffset,uiSize,&writed,pWriteData);
#else 
  ret=fseek(s->file,uiOffset,SEEK_SET);
  writed=fwrite(pWriteData,1,uiSize,s->file);
  fflush(s->file);
#endif
  printf("writed=0x%x return kVA_OK  fseek=%d\r\n",writed,ret);
  VA_XNVM_RequestDone(dwStbXnvmHandle);
  printf("VA_XNVM_RequestDone.\r\n");
  return kVA_OK;
}
#endif

/* End of File */
