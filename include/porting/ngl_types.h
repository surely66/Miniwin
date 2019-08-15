#ifndef __BASIC_TYPE_H__
#define __BASIC_TYPE_H__
#include<ngl_errors.h>

#ifdef  __cplusplus
#define NGL_BEGIN_DECLS extern "C" {
#define NGL_END_DECLS }
#else
#define NGL_BEGIN_DECLS
#define NGL_END_DECLS
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef HANDLE
#define HANDLE void*
#endif

#ifndef DWORD
#define DWORD unsigned long 
#endif

#ifndef LONG
#define LONG long
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef INT8
#define INT8 char
#endif

#ifndef UINT8
#define UINT8 unsigned char
#endif
 
#ifndef WORD
#define WORD unsigned short 
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef SHORT
#define SHORT short
#endif

#ifndef INT
#define INT int
#endif

#ifndef INT32
#define INT32 long
#endif

#ifndef UINT
#define UINT unsigned int
#endif

#ifndef UINT32 
#define UINT32 unsigned long
#endif

#ifndef UINT16
#define UINT16 unsigned short
#endif

#ifndef BOOL 
#define BOOL int
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL ((void*)0)
#endif


#endif
