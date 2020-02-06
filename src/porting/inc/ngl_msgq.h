#ifndef __NGL_MSGQ_H__
#define __NGL_MSGQ_H__
#include <ngl_types.h>

NGL_BEGIN_DECLS

HANDLE nglMsgQCreate(int howmany, int sizepermag);
DWORD nglMsgQDestroy(HANDLE msgid);
DWORD nglMsgQSend(HANDLE msgid, const void* pvmag, int msgsize, DWORD timeout);
DWORD nglMsgQReceive(HANDLE msgid, const void* pvmag, DWORD msgsize, DWORD timeout);
DWORD nglMsgQGetCount(HANDLE msgid,UINT*count);
NGL_END_DECLS

#endif
