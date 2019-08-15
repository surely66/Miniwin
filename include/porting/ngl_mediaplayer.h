#ifndef __NGL_MEDIA_PLAYER_H__
#define __NGL_MEDIA_PLAYER_H__
#include <ngl_types.h>

NGL_BEGIN_DECLS
DWORD nglMPOpen(const char*fname);
DWORD nglMPPlay(DWORD handle);
DWORD nglMPStop(DWORD handle);
DWORD nglMPClose(DWORD handle);
DWORD nglMPResume(DWORD handle);
DWORD nglMPPause(DWORD handle);

DWORD nglMPGetTime(DWORD handle,UINT*curtime,UINT*timems);
DWORD nglMPSeek(DWORD handle,UINT timems);
NGL_END_DECLS

#endif

