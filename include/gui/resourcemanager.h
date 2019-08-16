#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include<cairomm/surface.h>
#include <map>
#include <memory>

using namespace Cairo;
namespace nglui{

class ResourceManager{
private:
public:
  ResourceManager(const std::string&pakpath);
  RefPtr<ImageSurface>loadImage(const std::string&resname,bool cache=true);
  const std::string getString(const std::string&id,const std::string&lan="");
  size_t loadFile(const std::string&fname,unsigned char**buffer)const;
private:
  std::string osdlanguage;
  std::map<const std::string,RefPtr<ImageSurface>>images;
  std::map<const std::string,std::string>strings;
  std::unique_ptr<class FilePAK> pak;
//  RefPtr<ImageSurface>loadSVG(const char*xml,size_t size=0);
};

}//namespace
#endif
