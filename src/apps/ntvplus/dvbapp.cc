#include<dvbapp.h>
#include<ngl_tuner.h>
#include<ngl_dmx.h>
#include<ngl_video.h>
#include<dvbepg.h>
#include<satellite.h>
#include<ngl_log.h>
#include<favgroup.h>
NGL_MODULE(DVBAPP)
namespace nglui{


DVBApp::DVBApp(int argc,const char**argv)
  :App(argc,argv){
    nglTunerInit();
    nglDmxInit();
    nglAvInit();
    NGLOG_DEBUG("DVBApp::DVBApp");
    LoadSatelliteFromDB("satellites.json");
    DtvLoadProgramsData("dvb_programs.dat");
    FavInit("my_favorites.json");
    DtvInitLCN(1000);
}

}//namespace

