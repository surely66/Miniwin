#include<dvbapp.h>
#include<ngl_tuner.h>
#include<ngl_dmx.h>
#include<ngl_video.h>
#include<dvbepg.h>
#include<ngl_log.h>
#include<preferences.h>
#include<ngl_disp.h>
#include "client/linux/handler/exception_handler.h"
#include "client/linux/handler/minidump_descriptor.h"

extern void StartACS();
NGL_MODULE(DVBAPP)
namespace nglui{

static bool DumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
                  void* context,
                  bool succeeded) {
  printf("Dump path: %s\n", descriptor.path());
  return succeeded;
}

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
    pref.load("settings.pref");
    int res=pref.getInt("pciture","resolution",DISP_RES_720P);
    NGLOG_DEBUG("DVBApp::DVBApp resolution=%d",res);
    setOpacity(200);//getArgAsInt("alpha",255));    
    nglDispSetResolution(res);
    StartACS();    
    //google_breakpad::MinidumpDescriptor descriptor("/tmp");
    //eh=new google_breakpad::ExceptionHandler(descriptor, NULL, DumpCallback,NULL, true, -1);
}

}//namespace

