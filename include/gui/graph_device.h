#ifndef __GRAPH_DEVICE_H__
#define __GRAPH_DEVICE_H__
#include <rect.h>
#include <cairomm/context.h>
#include <map>

using namespace Cairo;
class GraphDevice{
private:
   int width;
   int height;
   struct FT_LibraryRec_*ft_library;
   std::map<std::string,RefPtr<const FontFace>>fonts;
   class GraphContext*primaryContext;//
   DWORD graphEvent;
   DWORD primarySurface;
   static std::vector<class GraphContext*>gSurfaces;
   static GraphDevice*mInst;
   static void ComposeProc(void*);
   GraphDevice(int format=-1);
   static bool CheckOverlapped(GraphContext*s,int idx);
public:
   static GraphDevice*getInstance();
   void getScreenSize(int &w,int&h);
   int getScreenWidth();
   int getScreenHeight(); 
   void flip(GraphContext*ctx);
   RefPtr<const FontFace>getFont(const std::string&family=std::string());
   GraphContext*createContext(int w,int h);
   GraphContext*createContext(const RECT&rect);
   GraphContext*getPrimaryContext();
   void remove(GraphContext*ctx);
};

#endif

