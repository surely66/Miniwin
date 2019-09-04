#ifndef __NGL_PVR_H__
#define __NGL_PVR_H__
#include<ngl_types.h>
#define PVR_MAX_AUDIO 8
#define PVR_MAX_NAME_LEN 512

NGL_BEGIN_DECLS

typedef struct{
    UINT recordMode;/*0--normal ,1--timeshifted*/
    UINT dmxid;
    USHORT video_pid;
    USHORT video_type;
    USHORT audio_pids[PVR_MAX_AUDIO];
    USHORT audio_types[PVR_MAX_AUDIO];
    USHORT pcr_pid;
    char folderName[PVR_MAX_NAME_LEN];
}NGLPVR_RECORD_PARAM;

DWORD nglPvrRecordOpen(const char*recorf_path,const NGLPVR_RECORD_PARAM*param);
DWORD nglPvrRecordClose(DWORD handle);
DWORD nglPvrRecordPause(DWORD handler);
DWORD nglPvrRecordResume(DWORD handler);

DWORD nglPvrPlayerOpen(const char*pvrpath);
DWORD nglPvrPlayerPlay(DWORD handle);
DWORD nglPvrPlayerStop(DWORD handle);
DWORD nglPvrPlayerPause(DWORD handle);
DWORD nglPvrPlayerClose(DWORD handle);

NGL_END_DECLS

#endif
