#include<dvbapp.h>
#include<ngl_tuner.h>
#include<ngl_dmx.h>
#include<ngl_video.h>
#include<dvbepg.h>
#include<satellite.h>
#include<ngl_log.h>
#include<favgroup.h>
#include<preferences.h>
#include<ngl_disp.h>

NGL_MODULE(DVBAPP)
namespace nglui{


DVBApp::DVBApp(int argc,const char**argv)
  :App(argc,argv){
    Preferences pref;
    nglTunerInit();
    nglDmxInit();
    nglAvInit();
    LoadSatelliteFromDB("satellites.json");
    DtvLoadProgramsData("dvb_programs.dat");
    FavInit("my_favorites.json");
    DtvInitLCN((LCNMODE)(LCN_FROM_BAT|LCN_FROM_USER),1000);
    pref.load("settings.pref");
    int res=pref.getInt("pciture","resolution",DISP_RES_720P);
    NGLOG_DEBUG("DVBApp::DVBApp resolution=%d",res);
    
    nglDispSetResolution(res);
    
}

}//namespace

