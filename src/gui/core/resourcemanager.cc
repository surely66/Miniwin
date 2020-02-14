#include<resourcemanager.h>
#include<filepak.h>
#include<cairomm/basicbitmap.h>
#include<algorithm>
#include<ngl_types.h>
#include<ngl_log.h>
#include<cairomm/context.h>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/memorystream.h>

NGL_MODULE(RESOURCEMANAGER)

RefPtr<ImageSurface> loadSVG(const char*buffer,size_t buffer_size);
RefPtr<ImageSurface>nano_loadSVG(const char*buffer,size_t buffer_size);
namespace nglui{

typedef struct {
   unsigned char*data;
   size_t size;
   size_t pos;
}PAKCLOSURE;

static cairo_status_t pak_read(void *closure,unsigned char *data,unsigned int length){
   PAKCLOSURE*p=(PAKCLOSURE*)closure;
   if(p->pos+length>p->size)
       length=p->size-p->pos;
   else if(p->pos>=p->size)
      return CAIRO_STATUS_READ_ERROR;
   memcpy(data,p->data+p->pos,length);
   p->pos+=length;
   return CAIRO_STATUS_SUCCESS; 
}

class map_value_finder{
public:
    map_value_finder(const std::string &cmp_string):m_s_cmp_string(cmp_string){}
    bool operator ()(const std::map<std::string,RefPtr<ImageSurface>>::value_type &pair){
        return pair.first == m_s_cmp_string;
    }
private:
    const std::string &m_s_cmp_string;                    
};

size_t ResourceManager::loadFile(const std::string&fname,unsigned char**buffer)const{
     PAKCLOSURE closure;
     closure.data=(unsigned char*)pak->getPAKEntryData(fname);
     closure.size=(size_t)pak->getPAKEntrySize(fname);
     closure.pos=0;
     *buffer=(unsigned char*)closure.data;
     return closure.size;      
} 

RefPtr<ImageSurface>ResourceManager::loadImage(const std::string&resname,bool cache){
     std::map<const std::string, RefPtr<ImageSurface> >::iterator it = images.end();
     it=std::find_if(images.begin(),images.end(),map_value_finder(resname));
     if(it!=images.end()){
         return it->second;
     }
     size_t pos=resname.rfind('.');
     std::string ext=resname.substr(pos+1);
     PAKCLOSURE closure;
     closure.data=(unsigned char*)pak->getPAKEntryData(resname);
     closure.size=(size_t)pak->getPAKEntrySize(resname); 
     closure.pos=0;
     NGLOG_DEBUG_IF(1||nullptr==closure.data,"name:%s data=%p size=%d  ext=%s",resname.c_str(),closure.data,closure.size,ext.c_str());
     RefPtr<ImageSurface>img;
     if(closure.data==nullptr)return img;
     if(ext=="svg"){
         //img=nano_loadSVG((const char*)closure.data,closure.size);
         //img=loadSVG((const char*)closure.data,closure.size);
     }else if(ext=="png"){
#ifdef CAIRO_HAS_PNG_FUNCTIONS
         img=ImageSurface::create_from_png(pak_read,&closure);
#endif
     }else if(ext=="jpg"||ext=="jpeg"){
         img=ImageSurface::create_from_jpg(pak_read,&closure);
     }else if(ext=="bmp"){
         BasicBitmap*bmp=BasicBitmap::LoadBmpFromMemory(closure.data,closure.size,NULL);//
         NGLOG_VERBOSE("bmp size=%dx%d fmt=%d pitch=%d",bmp->Width(),bmp->Height(),bmp->Format(),bmp->Pitch());
         if(bmp->Format()!=BasicBitmap::A8R8G8B8){
            BasicBitmap*cvt=new BasicBitmap(bmp->Width(), bmp->Height(),BasicBitmap::A8R8G8B8);
            if (cvt)
                cvt->Convert(0, 0, (const BasicBitmap*)bmp, 0, 0, bmp->Width(), bmp->Height());
            img=ImageSurface::create(cvt->Bits(),Surface::Format::ARGB32,cvt->Width(),cvt->Height(),cvt->Pitch());
            delete cvt;
         }else{
             NGLOG_DUMP("BMP",bmp->Bits(),8);
             img=ImageSurface::create(bmp->Bits(),Surface::Format::ARGB32,bmp->Width(),bmp->Height(),bmp->Pitch());
             delete bmp;
         }
     }
     if(cache)
         images.insert(std::map<const std::string,RefPtr<ImageSurface> >::value_type(resname,img));
     NGLOG_VERBOSE("image size=%dx%d",img->get_width(),img->get_height());
     return img;
}

const std::string ResourceManager::getString(const std::string& id,const std::string&lan){
     if((!lan.empty())&&(osdlanguage!=lan)){
         rapidjson::Document d;
         std::string resname="strings-";
         resname.append(lan);
         resname.append(".json");
         osdlanguage=lan;

         PAKCLOSURE closure;
         closure.data=(unsigned char*)pak->getPAKEntryData(resname);
         closure.size=(size_t)pak->getPAKEntrySize(resname);

         NGLOG_DEBUG("data=%p size=%d",closure.data,closure.size);
         NGLOG_ERROR_IF(closure.size==0||closure.data==NULL,"resource file %s not found",resname.c_str());
         rapidjson::MemoryStream ims((char*)closure.data,closure.size);
         d.ParseStream(ims);

         for (rapidjson::Value::MemberIterator m = d.MemberBegin(); m != d.MemberEnd(); ++m){
             strings[m->name.GetString()]=m->value.GetString();
             NGLOG_VERBOSE("%s:%s",m->name.GetString(),m->value.GetString());
         }
     }
     auto itr=strings.find(id);
     if(itr!=strings.end()&&!itr->second.empty()){
          return itr->second;
     }
     return id;
}

ResourceManager::ResourceManager(const std::string&pakpath){
    std::unique_ptr<FilePAK>pp(new FilePAK());
    pak=std::move(pp);
    pak->readPAK(pakpath);
}

}//namespace

