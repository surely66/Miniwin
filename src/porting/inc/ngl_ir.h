#ifndef __NGL_IR_H__
#define __NGL_IR_H__

#include <ngl_types.h>

NGL_BEGIN_DECLS

typedef enum{
   NGL_KEY_BACKSPACE=0x1003,
   NGL_KEY_ESCAPE   =0x1004,
   NGL_KEY_ENTER    =0x1005,
   NGL_KEY_DEL      =0x1006,
   NGL_KEY_MENU     =0x1007,
   NGL_KEY_EPG      =0x1008,
   NGL_KEY_SUBT     =0x1009,
   NGL_KEY_AUDIO    =0x100A,
   NGL_KEY_HELP     =0x1010,
   NGL_KEY_VOL_INC  =0x1011,
   NGL_KEY_VOL_DEC  =0x1012,
   NGL_KEY_CH_UP    =0x1013,
   NGL_KEY_CH_DOWN  =0x1014,

   NGL_KEY_LEFT    =0x1015,
   NGL_KEY_RIGHT   =0x1016, 
   NGL_KEY_UP      =0x1017,
   NGL_KEY_DOWN    =0x1018,
   NGL_KEY_PGUP    =0x1019,
   NGL_KEY_PGDOWN  =0x101A,
   NGL_KEY_POWER   =0x101B,
   NGL_KEY_MUTE    =0x101C,
  
   NGL_KEY_F0      =0x1030,
   NGL_KEY_F1      =0x1031,
   NGL_KEY_F2      =0x1032,
   NGL_KEY_F3      =0x1033,
   NGL_KEY_F4      =0x1034,
   NGL_KEY_F5      =0x1035,
   NGL_KEY_F6      =0x1036,
   NGL_KEY_F7      =0x1037,
   NGL_KEY_F8      =0x1038,
   NGL_KEY_F9      =0x1039,
   
   NGL_KEY_SPACE=0x20,

   NGL_KEY_0=0x30,
   NGL_KEY_1=0x31,
   NGL_KEY_2=0x32,
   NGL_KEY_3=0x33,
   NGL_KEY_4=0x34,
   NGL_KEY_5=0x35,
   NGL_KEY_6=0x36,
   NGL_KEY_7=0x37,
   NGL_KEY_8=0x38,
   NGL_KEY_9=0x39,
   
   NGL_KEY_A=0x41,
   NGL_KEY_B=0x42,
   NGL_KEY_C=0x43,
   NGL_KEY_D=0x44,
   NGL_KEY_E=0x45,
   NGL_KEY_F=0x46,
   NGL_KEY_G=0x47,
   NGL_KEY_H=0x48,
   NGL_KEY_I=0x49,
   NGL_KEY_J=0x4A,
   NGL_KEY_K=0x4B,
   NGL_KEY_L=0x4C,
   NGL_KEY_M=0x4D,
   NGL_KEY_N=0x4E,
   NGL_KEY_O=0x4F,
   NGL_KEY_P=0x50,
   NGL_KEY_Q=0x51,
   NGL_KEY_R=0x52,
   NGL_KEY_S=0x53,
   NGL_KEY_T=0x54,
   NGL_KEY_U=0x55,
   NGL_KEY_V=0x56,
   NGL_KEY_W=0x57,
   NGL_KEY_X=0x58,
   NGL_KEY_Y=0x59,
   NGL_KEY_Z=0x5A,

}NGLKEYCODE;

typedef enum{
   NGL_KEY_PRESSED=1,
   NGL_KEY_RELEASE=2
}NGLKEYSTATE;

typedef struct{
    NGLKEYSTATE state; 
    INT key_code;
    UINT repeat;
    DWORD event_time; 
}NGLKEYINFO;

typedef void(*NGLKEY_CALLBACK)(NGLKEYINFO*key,void*userdata);

DWORD nglIrInit();
DWORD nglIrOpen(int deviceid,const char*keymap);
DWORD nglIrRegisterCallback(DWORD handle,NGLKEY_CALLBACK cbk,void*data);
DWORD nglIrGetKey(DWORD handle,NGLKEYINFO*key,DWORD timeout);
DWORD nglIrClose(DWORD handle);

NGL_END_DECLS

#endif

