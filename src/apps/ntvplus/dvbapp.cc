#include<dvbapp.h>
#include<ngl_tuner.h>
#include<ngl_dmx.h>
#include<ngl_video.h>
#include<dvbepg.h>
#include<ngl_log.h>
#include<preferences.h>
#include<ngl_disp.h>
#include <ngl_ir.h>
#include <windowmanager.h>
#include "client/linux/handler/exception_handler.h"
#include "client/linux/handler/minidump_descriptor.h"
#include <mutex>

extern void StartACS();
NGL_MODULE(DVBAPP)
namespace nglui{

static bool DumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
                  void* context,
                  bool succeeded) {
  printf("Dump path: %s\n", descriptor.path());
  return succeeded;
}

class IREventSource:public looper::EventSource{
public:
    std::mutex mtx;
private:
    DWORD ir_handle_;
    std::vector<NGLKEYINFO>keys; 
    static void KEYCBK(NGLKEYINFO*k,void*p){
        NGLOG_VERBOSE("key=0x%x/%d",k->key_code,k->key_code);
        IREventSource*es=(IREventSource*)p;
        std::lock_guard<std::mutex> lck (es->mtx);
        es->keys.emplace(es->keys.begin(),*k); 
    }
public:
    IREventSource(){
        nglIrInit();
        ir_handle_=nglIrOpen(0,nullptr);
        nglIrRegisterCallback(ir_handle_,KEYCBK,this);
    }
    bool prepare(int&) override { return keys.size()>0; }
    bool check(){
        return keys.size()>0;
    }
    bool dispatch(EventHandler &func) { return func(*this); }
    bool is_file_source() const override final { return false; }
    bool processKey(){
       std::lock_guard<std::mutex> lck (mtx);
       if(keys.size()==0)return false;
       NGLKEYINFO key=keys.back(); 
       keys.pop_back(); 
       if(key.state==NGL_KEY_PRESSED)
          WindowManager::getInstance()->onKeyPress(key.key_code);
       else
          WindowManager::getInstance()->onKeyRelease(key.key_code);
       return true;
    }
};

google_breakpad::ExceptionHandler *eh;
DVBApp::DVBApp(int argc,const char**argv)
  :App(argc,argv){
    Preferences pref;

    nglGraphInit();
    DtvEpgInit();
    LoadSatelliteFromDB("satellites.json");
    DtvLoadProgramsData("dvb_programs.dat");
    FavInit(getArg("favorite","favorites.json").c_str());
    DtvInitLCN((LCNMODE)(LCN_FROM_BAT|LCN_FROM_USER),1000);
    LoadServiceAdditionals("additionals.json");
    pref.load("settings.pref");
    int res=pref.getInt("pciture","resolution",DISP_RES_720P);
    NGLOG_DEBUG("DVBApp::DVBApp resolution=%d",res);
    setOpacity(200);//getArgAsInt("alpha",255));    
    nglDispSetResolution(res);
    addEventSource(new IREventSource(),[](looper::EventSource&s){
        return ((IREventSource&)s).processKey();
    });
    StartACS();    
    //google_breakpad::MinidumpDescriptor descriptor("/tmp");
    //eh=new google_breakpad::ExceptionHandler(descriptor, NULL, DumpCallback,NULL, true, -1);
}

}//namespace

