#ifndef __APPLICATION_H__
#define __APPLICATION_H__
#include<string>
#include<map>
#include<cairomm/surface.h>
#include<looper/looper.h>
using namespace Cairo;
using namespace looper;
namespace nglui{

class App{
private:
    class ResourceManager*resmgr;
    ResourceManager*getResourceManager();
    std::map<std::string,std::string>args;
    looper::EventLoop looper;
protected:
    static App*mInst;
    std::string name;
public:
     App(int argc,const char*argv[]);
     static App&getInstance();
     void setOpacity(unsigned char alpha);
     void setName(const std::string&appname);
     const std::string&getName();

     RefPtr<ImageSurface>loadImage(const std::string&resname,bool cache=true);

     const std::string getString(const std::string&id,const std::string&lan="");

     size_t loadFile(const std::string&fname,unsigned char**buffer)const;
     void setArg(const std::string&key,const std::string&value);
     bool hasArg(const std::string&key);
     const std::string&getArg(const std::string&key,const std::string&def="");

     int getArgAsInt(const std::string&key,int def);
     int addEventSource(EventSource *source, EventHandler handler);
     int removeEventSource(EventSource*source);
     int exec();
};

}//namespace
#endif
