/*
  FILE : stub_ctrl.c
  PURPOSE: This file is a stub for linking tests.
*/

#include <stdio.h>

#if 1//!defined VAOPT_DISABLE_VA_CTRL && !defined(__STB_SIMULATOR__)

#include "va_ctrl.h"
extern UINT16 g_service_id;
INT VA_CTRL_GetProgramInformation( DWORD dwAcsId, tVA_CTRL_ProgramInformation *pProgramInfo )
{
  //printf("%s\r\n",__FUNCTION__);
  pProgramInfo->wNetworkId=0x500;
  pProgramInfo->wTransportStreamId=0x1000;
  pProgramInfo->wServiceId=g_service_id;
  return 0;
}

typedef struct{
    DWORD dwAcsId;
    tVA_CTRL_AcsMode acsMode;
}NGL_ACS;
static NGL_ACS sACS[kVA_SETUP_NBMAX_ACS];

INT NGL_SetAcsMode(DWORD dwAcsId,tVA_CTRL_AcsMode eAcsMode)
{
   sACS[dwAcsId].acsMode=eAcsMode;
   return kVA_OK;
}

INT  NGL_GetAcsMode(DWORD dwAcsId,tVA_CTRL_AcsMode*eAcsMode){
   *eAcsMode=sACS[dwAcsId].acsMode;
   return kVA_OK;
}

#else /* !defined VAOPT_DISABLE_VA_CTRL && !defined(__STB_SIMULATOR__) */

extern void STUB_CTRL_Dummy(void);
void STUB_CTRL_Dummy(void) {
  printf("%s\r\n",__FUNCTION__);
}

#endif /* !defined VAOPT_DISABLE_VA_CTRL && !defined(__STB_SIMULATOR__) */

/* End of File */
