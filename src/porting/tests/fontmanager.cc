#include <fontmanager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ngl_types.h>
#include <ngl_log.h>
#include <canvas.h>
NGL_MODULE(FONTMANAGER)

namespace nglui{
    //http://www.verysource.com/code/5938830_1/GUI_TTF.c.html
    FontManager*FontManager::mInst=nullptr;

    FT_Error FontManager::FaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface){
        FontManager*fm=(FontManager*)request_data;
        Canvas::LOGFONT*lf=(Canvas::LOGFONT*)face_id;
        std::string fntfile=fm->families.begin()->second;
        if(lf&&strlen(lf->lfFaceName)){
            std::map<std::string,std::string>::iterator it=fm->families.find(lf->lfFaceName);///usr/lib/fonts/droid_chn.ttf
            if(it!=fm->families.end())
                fntfile=it->second;
        }
        return FT_New_Face(fm->library,fntfile.c_str(), 0, aface);
    }

    FontManager::FontManager(){
        FT_Init_FreeType(&library);
        FTC_Manager_New(library,8,0,0,FaceRequester,(FT_Pointer)this, &cache_manager);
        FTC_ImageCache_New(cache_manager,&img_cache);
        FTC_CMapCache_New(cache_manager, &cmap_cache);
        FTC_SBitCache_New(cache_manager, &sbit_cache);
    }
     
    FontManager&FontManager::getInstance(){
        if(mInst==nullptr)
            mInst=new FontManager();
        return *mInst;
    }

    int FontManager::loadFonts(const char*path){
        int rc=0;
        DIR *dir=opendir(path);
        struct dirent *entry;
        NGLOG_DEBUG("fontpath=%s dir=%p",path,dir);
        while(dir&&(entry=readdir(dir))){
            FT_Face face;
            struct stat st;
            std::string s=path;
            s.append("/");
            s.append(entry->d_name);
            if(stat(s.c_str(),&st)==-1)continue;
            if( (S_ISREG(st.st_mode)) && (FT_New_Face(library,s.c_str(),0,&face)==FT_Err_Ok) ){
                families[face->family_name]=s;
                NGLOG_DEBUG("family:%s style:%s file:%s glyphs:%d",face->family_name,face->style_name,s.c_str(),face->num_glyphs);
                FT_Done_Face(face);
                rc++;
            }
        }
        if(dir)
            closedir(dir);
        return rc;  
    }

    int FontManager::loadFonts(const char**dirs,int count){
        int rc=0;
        for(int i=0;i<count;i++){
            FT_Face face;
            if(FT_New_Face(library,dirs[i],0,&face)==FT_Err_Ok){
                families[face->family_name]=dirs[i];
                FT_Done_Face(face);
                rc++;
            }
        }
        return rc;
    }

    FT_Error FontManager::getFace(FTC_FaceID face_id,FT_Face*aface){
        FT_Error err;
        FT_Size size;
        FTC_ScalerRec_ scaler;
        Canvas::LOGFONT*lf=(Canvas::LOGFONT*)face_id;
        //err=FTC_Manager_LookupFace(cache_manager,face_id,aface);
        scaler.face_id = face_id;
        scaler.width   = lf->lfWidth;
        scaler.height  = lf->lfHeight;
        scaler.pixel   = 1;
        err=FTC_Manager_LookupSize(cache_manager, &scaler, &size);
        *aface=size->face;
        return err;
    }
    unsigned int FontManager::getGraphIndex(FTC_FaceID face_id,unsigned int c){
        FT_Face face;
        getFace(face_id,&face);
        for (unsigned i=0; i<face->num_charmaps; i++){  
           if(face->charmaps[i]->encoding == FT_ENCODING_UNICODE){  
              return  FTC_CMapCache_Lookup(cmap_cache,face_id, i, c);  
           }   
        }  
        return FTC_CMapCache_Lookup(cmap_cache,face_id, 0, c); 
    }
    
    FT_Error FontManager::getGlyph(FTC_FaceID face_id,unsigned int gidx,FT_Glyph*glyph,int loadflags){
        FTC_Node node;
        FTC_ScalerRec_ scaler;
        Canvas::LOGFONT*lf=(Canvas::LOGFONT*)face_id;
        scaler.face_id = face_id;
        scaler.width   = lf->lfWidth;
        scaler.height  = lf->lfHeight;
        scaler.pixel   = 1;
        return FTC_ImageCache_LookupScaler(img_cache,&scaler,loadflags,gidx,glyph,&node);
    }
    
    FT_Error FontManager::getCharBitmap(FTC_FaceID face_id,unsigned int gidx,FTC_SBit* sbit){
        FTC_ScalerRec_ scaler;
        FTC_Node node;
        Canvas::LOGFONT*lf=(Canvas::LOGFONT*)face_id;
        scaler.face_id = face_id;
        scaler.width   = lf->lfWidth;
        scaler.height  = lf->lfHeight;
        scaler.pixel   = 1;
        
        return FTC_SBitCache_LookupScaler(sbit_cache,&scaler,FT_LOAD_RENDER,gidx,sbit,&node);
    }
}//namespace
