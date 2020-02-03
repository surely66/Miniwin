#ifndef __FONT_MANAGER_H__
#define __FONT_MANAGER_H__
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftcache.h>
#include <ngl_types.h>
#include <map>

namespace nglui{

class FontManager{
private:
   FT_Library  library;
   FTC_Manager cache_manager;
   FTC_CMapCache cmap_cache;
   FTC_ImageCache img_cache;
   FTC_SBitCache sbit_cache;
   std::map<std::string,std::string>families;
   static FontManager*mInst;
   static FT_Error FaceRequester(FTC_FaceID,FT_Library,FT_Pointer,FT_Face*);
   FontManager();
public:
   static FontManager&getInstance();
   FT_Library getLibrary(){return library;}
   int loadFonts(const char*dir);
   int loadFonts(const char**dirs,int count);
   FT_Error getFace(FTC_FaceID face_id,FT_Face*aface);
   unsigned int getGraphIndex(FTC_FaceID face,unsigned int c);
   FT_Error getGlyph(FTC_FaceID face_id,unsigned int gidx,FT_Glyph*glyph,int loadflags=0);
   FT_Error getCharBitmap(FTC_FaceID face_id,unsigned int gidx,FTC_SBit*sbit);
};
}//namespace
#endif

