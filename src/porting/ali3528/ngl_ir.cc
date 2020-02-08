#include<ngl_ir.h>
#include<aui_input.h>
#include<map>
#include<iostream>
#include<fstream>
#include<ngl_log.h>
#include<ngl_msgq.h>
#include<ngl_timer.h>

NGL_MODULE(IRINPUT)

static aui_hdl hdl_key=NULL;

typedef struct{
   aui_hdl hdl;
   std::map<int,int>keymap;
   NGLKEY_CALLBACK cbk;
   void*userdata;
}IRDEV;

#define NB_DEV 8

static IRDEV irdevices[NB_DEV];
static std::map<std::string,int>name2key;
static std::map<int,std::string>key2name;
static DWORD msgQ=0;
#define REGIST_KEY(x) {name2key[#x]=NGL_##x ;key2name[NGL_##x]=#x;}

DWORD nglIrInit(){
    if(msgQ)return 0;
    msgQ=nglMsgQCreate(16,sizeof(aui_key_info));
    aui_key_init(NULL,NULL);
    for(int i=0;i<NB_DEV;i++)irdevices[i].hdl=NULL;
    aui_log_priority_set(AUI_MODULE_INPUT,AUI_LOG_PRIO_DEBUG);    
    aui_log_priority_set(AUI_MODULE_PANEL,AUI_LOG_PRIO_DEBUG);    
    //REGIST_KEY(KEY_LEFT);it the same as keynames["KEY_LEFT"]=NGL_KEY_LEFT;
    REGIST_KEY(KEY_POWER);
    REGIST_KEY(KEY_BACKSPACE);
    REGIST_KEY(KEY_ESCAPE);
    REGIST_KEY(KEY_ENTER);
    REGIST_KEY(KEY_DEL);
    REGIST_KEY(KEY_MENU);
    REGIST_KEY(KEY_EPG);
    REGIST_KEY(KEY_SUBT); 
    REGIST_KEY(KEY_AUDIO);
    REGIST_KEY(KEY_HELP);
    REGIST_KEY(KEY_VOL_INC);
    REGIST_KEY(KEY_VOL_DEC);
    REGIST_KEY(KEY_CH_UP);
    REGIST_KEY(KEY_CH_DOWN);

    REGIST_KEY(KEY_LEFT);
    REGIST_KEY(KEY_RIGHT);
    REGIST_KEY(KEY_UP); 
    REGIST_KEY(KEY_DOWN);
    REGIST_KEY(KEY_PGUP);
    REGIST_KEY(KEY_PGDOWN);
     
    REGIST_KEY(KEY_0);    REGIST_KEY(KEY_1);
    REGIST_KEY(KEY_2);    REGIST_KEY(KEY_3);    REGIST_KEY(KEY_4);    REGIST_KEY(KEY_5);
    REGIST_KEY(KEY_6);    REGIST_KEY(KEY_7);    REGIST_KEY(KEY_8);    REGIST_KEY(KEY_9);

    REGIST_KEY(KEY_F0);   REGIST_KEY(KEY_F1);
    REGIST_KEY(KEY_F2);   REGIST_KEY(KEY_F3);    REGIST_KEY(KEY_F4);   REGIST_KEY(KEY_F5);
    REGIST_KEY(KEY_F6);   REGIST_KEY(KEY_F7);    REGIST_KEY(KEY_F8);   REGIST_KEY(KEY_F9);
    
    REGIST_KEY(KEY_A);    REGIST_KEY(KEY_B);    REGIST_KEY(KEY_C);    REGIST_KEY(KEY_D);
    REGIST_KEY(KEY_E);    REGIST_KEY(KEY_F);    REGIST_KEY(KEY_G);    REGIST_KEY(KEY_H);
    REGIST_KEY(KEY_I);    REGIST_KEY(KEY_J);    REGIST_KEY(KEY_K);    REGIST_KEY(KEY_L);
    REGIST_KEY(KEY_N);    REGIST_KEY(KEY_O);    REGIST_KEY(KEY_P);    REGIST_KEY(KEY_Q);
    REGIST_KEY(KEY_R);    REGIST_KEY(KEY_S);    REGIST_KEY(KEY_T);    REGIST_KEY(KEY_U);
    REGIST_KEY(KEY_V);    REGIST_KEY(KEY_W);    REGIST_KEY(KEY_X);    REGIST_KEY(KEY_Y);
    REGIST_KEY(KEY_Z);
    for(auto k=name2key.begin();k!=name2key.end();k++)
        NGLOG_VERBOSE("[%s]  %x",k->first.c_str(),k->second);

}

static void key_callback(aui_key_info*ki,void*d){
    int rc=nglMsgQSend(msgQ,ki,sizeof(aui_key_info),100);
    NGLKEYINFO ko;
    IRDEV*ir=&irdevices[0];
    auto fnd=ir->keymap.find(ki->n_key_code);
    ko.key_code=(fnd!=ir->keymap.end())?fnd->second:ki->n_key_code;
    ko.repeat=ki->n_count;
    ko.state=ki->e_status==1?NGL_KEY_PRESSED:NGL_KEY_RELEASE;
    NGL_RunTime rt;
    nglGetRunTime(&rt);
    ko.event_time=rt.uiMilliSec;
    if(ir->cbk)ir->cbk(&ko,ir->userdata); 
}

HANDLE nglIrOpen(int id,const char*keymap){
    IRDEV*ir=&irdevices[id];
    std::ifstream fin;
    fin.open(keymap?keymap:"key.map");
    char line[256];
    
    int rc=aui_key_open(0,NULL,&ir->hdl);
    rc= aui_key_set_ir_rep_interval(ir->hdl,100,100);//default is 600 350
    rc=aui_key_callback_register(ir->hdl,key_callback);
#define SEP " \t,;:"
    while(fin.is_open()&&fin.getline(line,256)){
//read keymap format as KEY_NAME ,key_code
        int idx=0;
        char *p,*ps[32];
        p=strtok(line,SEP);
        while(p&&idx<32){
            ps[idx++]=p;
            p=strtok(NULL,SEP);
        }
       if(idx<2||NULL==strstr(ps[0],"KEY"))continue;
       int base=strncasecmp(ps[1],"0x",2)==0?16:10;
       long key=strtoll(ps[1],NULL,base);
       std::string kname(ps[0]);
       NGLOG_VERBOSE("keymap[%s],0x%x",kname.c_str(),key);
       NGLOG_VERBOSE("nametokey [%s]-->%x",kname.c_str(),name2key[kname]);
       ir->keymap[key]=name2key[kname];
    }
    if(rc==0)return (DWORD)ir;
    return 0;
}

DWORD nglIrRegisterCallback(HANDLE handle,NGLKEY_CALLBACK cbk,void*data){
    IRDEV*ir=(IRDEV*)handle;
    ir->cbk=cbk;
    ir->userdata=data;
    return 0;
}

DWORD nglIrGetKey(HANDLE handle,NGLKEYINFO*key,DWORD timeout){
    IRDEV*ir=(IRDEV*)handle;
    aui_key_info info;

    if(ir<irdevices||ir>=&irdevices[NB_DEV]){
        NGLOG_DEBUG("invalid para %p [%p,%p]",ir,&irdevices[0],&irdevices[NB_DEV]);
        return NGL_INVALID_PARA;
    }
    memset(key,0,sizeof(NGLKEYINFO));
    memset(&info,9,sizeof(aui_key_info));
#if 0
    int rc=aui_key_key_get(ir->hdl,&info);
#else
    int rc= nglMsgQReceive(msgQ,&info,sizeof(aui_key_info),timeout);
#endif
    auto fnd=ir->keymap.find(info.n_key_code);
    key->key_code=(fnd!=ir->keymap.end())?fnd->second:info.n_key_code;
    key->repeat=info.n_count;
    key->state=info.e_status==1?NGL_KEY_PRESSED:NGL_KEY_RELEASE;
    NGL_RunTime rt;
    nglGetRunTime(&rt);
    key->event_time=rt.uiMilliSec;
    NGLOG_VERBOSE_IF(rc==0,"key_code=0x%x<--0x%x state=%d  repeat=%d keyname=%s",key->key_code,info.n_key_code,
         key->state,key->repeat, key2name[key->key_code].c_str());
    key->state=(NGLKEYSTATE)info.e_status;
    return rc==0?NGL_OK:NGL_ERROR;
}

DWORD nglIrClose(HANDLE handle){
    IRDEV*ir=(IRDEV*)handle;
    if(ir<irdevices||ir>=&irdevices[NB_DEV])
        return NGL_INVALID_PARA;
    aui_key_close(ir->hdl);
    ir->hdl=NULL;
    ir->keymap.clear();
    return NGL_OK;
}

