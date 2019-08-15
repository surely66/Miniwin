#ifndef __NGL_MSGQ_H__
#define __NGL_MSGQ_H__
#include <ngl_types.h>

NGL_BEGIN_DECLS

DWORD nglMsgQCreate(int howmany, int sizepermag);
DWORD nglMsgQDestroy(DWORD msgid);
DWORD nglMsgQSend(DWORD msgid, const void* pvmag, int msgsize, DWORD timeout);
DWORD nglMsgQReceive(DWORD msgid, const void* pvmag, DWORD msgsize, DWORD timeout);
DWORD nglMsgQGetCount(DWORD msgid,UINT*count);
NGL_END_DECLS

#endif
