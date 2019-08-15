#ifndef __APPLICATION_H__
#define __APPLICATION_H__
#include<string>
#include<cairomm/surface.h>
using namespace Cairo;
namespace nglui{

class Application{
private:
    class ResourceManager*resmgr;
    ResourceManager*getResourceManager();
protected:
    static Application*mInst;
    std::string name;
public:
     Application(int argc,const char*argv[]);
     static Application&getInstance();
     void setOpacity(unsigned char alpha);
     void setName(const std::string&appname);
     const std::string&getName();
     RefPtr<ImageSurface>loadImage(const std::string&resname,bool cache=true);
     const std::string getString(const std::string&id,const char*lan=nullptr);
     size_t loadFile(const std::string&fname,unsigned char**buffer)const;

     int exec();
};

}//namespace
#endif
