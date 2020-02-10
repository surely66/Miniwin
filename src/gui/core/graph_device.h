#ifndef __GRAPH_DEVICE_H__
#define __GRAPH_DEVICE_H__
#include <rect.h>
#include <cairomm/context.h>
#include <map>

using namespace Cairo;
namespace nglui{
class GraphDevice{
private:
   int width;
   int height;
   struct FT_LibraryRec_*ft_library;
   std::map<std::string,RefPtr<const FontFace>>fonts;
   class GraphContext*primaryContext;//
   DWORD compose_event;
   HANDLE primarySurface;
   static std::vector<class GraphContext*>gSurfaces;
   static GraphDevice*mInst;
   GraphDevice(int format=-1);
   static bool CheckOverlapped(GraphContext*s,int idx);
public:
   static GraphDevice*getInstance();
   void getScreenSize(int &w,int&h);
   int getScreenWidth();
   int getScreenHeight(); 
   void flip(GraphContext*ctx);
   void ComposeSurfaces();
   bool needCompose(){return compose_event;}
   RefPtr<const FontFace>getFont(const std::string&family=std::string());
   GraphContext*createContext(int w,int h);
   GraphContext*createContext(const RECT&rect);
   GraphContext*getPrimaryContext();
   void remove(GraphContext*ctx);
};
}
#endif

