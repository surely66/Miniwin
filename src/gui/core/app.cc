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
#include <fcntl.h>
#include <thread>
#include <ngl_ir.h>
#include <windowmanager.h>
#include <mutex>

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
class UIEventSource:public looper::EventSource{
public:
    UIEventSource(){};
    int getEvents(){return WindowManager::getInstance()->hasEvents();}
    bool prepare(int&) override { return getEvents();}
    bool check(){
        return  getEvents();
    }
    bool dispatch(EventHandler &func) { return func(*this); }

};
class IREventSource:public looper::EventSource{
public:
    std::mutex mtx;
private:
    HANDLE ir_handle_;
    std::vector<NGLKEYINFO>keys;
    static void KEYCBK(NGLKEYINFO*k,void*p){
        NGLOG_DEBUG("key=0x%x/%d",k->key_code,k->key_code);
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
App::App(int argc,const char*argv[]){
    struct stat pakstat;
    int option_index,c;
    nglLogParseModules(argc,argv);    
    resmgr=nullptr;
    mInst=this;
    setName(argv[0]);
    if(argc&&argv){
       spt_init(argc,(char**)argv);
    }
    nglGraphInit();

    USBManager::getInstance().startMonitor();
    do{
        c=getopt_long(argc,(char*const*)argv,"a",app_options,&option_index);
        if(c>=0){
            std::string key=app_options[option_index].name;
            args[key]=optarg;
            NGLOG_VERBOSE("args[%d]%s:%s",option_index,key.c_str(),optarg);
        }
    }while(c>=0);
    setOpacity(getArgAsInt("alpha",255));
    addEventSource(new IREventSource(),[](looper::EventSource&s){
        return ((IREventSource&)s).processKey();
    });
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

bool App::hasArg(const std::string&key){
    auto itr=args.find(key);
    return itr==args.end();
}

void App::setArg(const std::string&key,const std::string&value){
    args[key]=value;
}

int App::getArgAsInt(const std::string&key,int def){
    auto itr=args.find(key);
    if(itr==args.end()||itr->second.empty())
        return def;
    return std::stol(args[key]);
}

void App::setOpacity(unsigned char alpha){
    nglSurfaceSetOpacity(0,alpha);
    NGLOG_DEBUG("alpha=%d",alpha);
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

int App::addEventSource(EventSource *source, EventHandler handler){
    return looper.add_event_source(source,handler);
}

int App::removeEventSource(EventSource*source){
    return looper.remove_event_source(source);
}

int App::exec(){
    looper.add_event_source(new UIEventSource(),[](EventSource&e)->bool{
        WindowManager::getInstance()->runOnce();
        return true;
    });
    /*for(int i=0;i<2;i++){
       char fname[32];
       sprintf(fname,"/dev/input/event%d",i);
       int fd=open(fname,O_RDWR|O_NONBLOCK);
       looper.add_event_source(new InputEventSource(fd),[](EventSource&s){
           InputEventSource& fe=(InputEventSource&)s;
           struct input_event e;
           fe.getEvent(e);
           fe.dumpEvent(e);
           return true;
       });
    }*/
    looper.run();
    //std::thread thread_loop(std::bind(&looper::EventLoop::run,&looper));
    //thread_loop.join();
}

void App::setName(const std::string&appname){
    //setproctitle(appname.c_str());
    name=appname;
    getResourceManager();
}

const std::string& App::getName(){
    return name;
}

}

