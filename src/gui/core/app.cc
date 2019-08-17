#include <app.h>
#include <ngl_types.h>
#include <ngl_log.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <windowmanager.h>
#include <resourcemanager.h>
#include <cairomm/surface.h>
#include <usbmanager.h>
#include <getopt.h>

NGL_MODULE(APP)

void spt_init(int argc, char *argv[]);
void setproctitle(const char *fmt, ...);
namespace nglui{

App*App::mInst=nullptr;

static struct option app_options[]={
   {"alpha",required_argument,0,0},
   {"config",required_argument,0,0},
   {"language",required_argument,0,0},
   {0,0,0,0}
};
App::App(int argc,const char*argv[]){
    struct stat pakstat;
    int option_index,c;
    nglLogParseModules(argc,argv);    
    resmgr=nullptr;
    mInst=this;
    spt_init(argc,(char**)argv);
    setName(argv[0]);
    USBManager::getInstance().startMonitor();
    do{
        c=getopt_long(argc,(char*const*)argv,"a",app_options,&option_index);
        if(c>=0){
            std::string key=app_options[option_index].name;
            args[key]=optarg;
            NGLOG_VERBOSE("args[%d]%s:%s",option_index,key.c_str(),optarg);
        }
    }while(c>=0);
}
App&App::getInstance(){
    return *mInst;
}

const std::string&App::getArg(const std::string&key,const std::string&def){
    auto itr=args.find(key);
    if(itr==args.end()||itr->second.empty())
        return def;
    return args[key];
}

int App::getArgAsInt(const std::string&key,int def){
    auto itr=args.find(key);
    if(itr==args.end()||itr->second.empty())
        return def;
    return std::stol(args[key]);
}
void App::setOpacity(unsigned char alpha){
    nglSurfaceSetOpacity(0,alpha);
}

ResourceManager*App::getResourceManager(){
    if(resmgr==nullptr){
        struct stat pakstat;
        std::string pakname=getName();
        pakname+=".pak";
        NGLOG_DEBUG("resname=%s",pakname.c_str());
        int rc=stat(pakname.c_str(),&pakstat);
        if(rc==0)
           resmgr=new ResourceManager(pakname);
        NGLOG_DEBUG("");
    }
    return resmgr;
}

RefPtr<ImageSurface>App::loadImage(const std::string&resname,bool cache){
    if(resmgr)return resmgr->loadImage(resname,cache);
    return RefPtr<ImageSurface>(nullptr);
}

const std::string App::getString(const std::string&id,const std::string&lan){
    if(resmgr)return resmgr->getString(id,lan);
    return std::string();
}

size_t App::loadFile(const std::string&fname,unsigned char**buffer)const{
    if(resmgr)return resmgr->loadFile(fname,buffer);
    return 0;
}


int App::exec(){
    //WindowManager::getInstance()->run();
    while(1){
       nglSleep(1000);
    }
}
void App::setName(const std::string&appname){
    setproctitle(appname.c_str());
    name=appname;
    getResourceManager();
}
const std::string& App::getName(){
    return name;
}

}
