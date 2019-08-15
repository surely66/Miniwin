#include<resourcemanager.h>
#include<filepak.h>
#include<cairomm/basicbitmap.h>
#include<algorithm>
#include<ngl_types.h>
#include<ngl_log.h>
#include<cairomm/context.h>
#include<json/json.h>
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
#if 0
static void RenderSVGPath(const RefPtr<Context>&cr,ssvg::Path&p){
    cr->begin_new_path();
    for(int i=0;i<p.m_NumCommands;i++){
        ssvg::PathCmd&cmd=p.m_Commands[i];
        float*d=cmd.m_Data;
        switch(cmd.m_Type){
        case ssvg::PathCmdType::MoveTo : cr->move_to(cmd.m_Data[0],cmd.m_Data[1]);break;
        case ssvg::PathCmdType::LineTo : cr->line_to(cmd.m_Data[0],cmd.m_Data[1]);break;
        case ssvg::PathCmdType::CubicTo:
             cr->curve_to(d[0],d[1],d[2],d[3],d[4],d[5]);break;
        case ssvg::PathCmdType::QuadraticTo:
        case ssvg::PathCmdType::ArcTo  :NGLOG_DEBUG("todo for type %d",cmd.m_Type);break;
        case ssvg::PathCmdType::ClosePath: cr->close_path();break;
        }
    }
}

static void SetAttr(const RefPtr<Context>&cr,ssvg::ShapeAttributes&attr){
    
    cr->set_line_width(attr.m_StrokeWidth);
    cr->set_line_cap((Cairo::LineCap)attr.m_StrokeLineCap);
    cr->set_line_join((Cairo::LineJoin)attr.m_StrokeLineJoin);
    cr->set_miter_limit(attr.m_StrokeMiterLimit);
    cr->set_font_size(attr.m_FontSize);
    UINT c=attr.m_FillPaint.m_ColorABGR;
    cr->set_source_rgba((c&0xFF)/255. , ((c>>8)&0xFF)/255. , ((c>>16)&0xFF)/255.,attr.m_FillOpacity);
    switch(attr.m_FillPaint.m_Type){
    case ssvg::PaintType::None:
    case ssvg::PaintType::Transparent:break;
    case ssvg::PaintType::Color:cr->fill_preserve();break;
    }
 
    c=attr.m_StrokePaint.m_ColorABGR;
    cr->set_source_rgba((c&0xFF)/255. , ((c>>8)&0xFF)/255. , ((c>>16)&0xFF)/255.,attr.m_StrokeOpacity);
    switch(attr.m_StrokePaint.m_Type){
    case ssvg::PaintType::None:
    case ssvg::PaintType::Transparent:break;
    case ssvg::PaintType::Color:cr->stroke();
    }
}

static void RenderShape(const RefPtr<Context>&cr,ssvg::Shape&shape ){
    float*f=shape.m_Attrs.m_Transform;
    cairo_matrix_t m={f[0],f[1],f[2],f[3],f[4],f[5]};
    cr->transform(m);
    switch(shape.m_Type){
    case ssvg::ShapeType::Group:{
             ssvg::ShapeList&grp=shape.m_ShapeList;
             for(int i=0;i<grp.m_NumShapes;i++)
                  RenderShape(cr,grp.m_Shapes[i]);
         }break;
    case ssvg::ShapeType::Rect:{
             ssvg::Rect&r=shape.m_Rect;
             cr->rectangle(r.x,r.y,r.width,r.height);
         }break;
    case ssvg::ShapeType::Circle:{
             ssvg::Circle&c=shape.m_Circle;
             cr->arc(c.cx,c.cy,c.r,.0f,2*M_PI);
         }break;
    case ssvg::ShapeType::Ellipse:{
         }break;
    case ssvg::ShapeType::Line:{
             ssvg::Line&l=shape.m_Line;
             cr->move_to(l.x1,l.y1);
             cr->line_to(l.x2,l.y2);
         }break;
    case ssvg::ShapeType::Polyline:{
             ssvg::PointList&p=shape.m_PointList;
             cr->move_to(p.m_Coords[0],p.m_Coords[1]);
             for(int i=1;i<p.m_NumPoints;i++)
                 cr->line_to(p.m_Coords[i+i],p.m_Coords[i+i+1]);
         }break;
    case ssvg::ShapeType::Polygon:{
             ssvg::PointList&p=shape.m_PointList;
             cr->move_to(p.m_Coords[0],p.m_Coords[1]);
             for(int i=1;i<p.m_NumPoints;i++)
                 cr->line_to(p.m_Coords[i+i],p.m_Coords[i+i+1]);
             cr->line_to(p.m_Coords[0],p.m_Coords[1]);
         }break;
    case ssvg::ShapeType::Path:
          RenderSVGPath(cr,shape.m_Path);
          break;
    case ssvg::ShapeType::Text:{
            ssvg::Text&t=shape.m_Text;
            cr->move_to(t.x,t.y);
            NGLOG_DEBUG("ShapeType::Text %s",t.m_String);
            cr->text_path(t.m_String);
         }break;
    default:break;
    }
    SetAttr(cr,shape.m_Attrs);    
}
static void RenderSVG(const RefPtr<Context>&cr,ssvg::Image&img ){
    for(int i=0;i<img.m_ShapeList.m_NumShapes;i++){
        RenderShape(cr,img.m_ShapeList.m_Shapes[i]);
    }
}
RefPtr<ImageSurface>ResourceManager::loadSVG(const char*xmlstr,size_t size){
    ssvg::Image*svgimg=ssvg::imageLoad(xmlstr);
    UINT w=svgimg->m_ViewBox[2],h=svgimg->m_ViewBox[3];
    NGLOG_DEBUG("loadSVG size=%dx%d  \r\n %s",w,h,xmlstr);
    const RefPtr<ImageSurface>image=ImageSurface::create(FORMAT_ARGB32,w,h);//svgimg->m_Width,svgimg->m_Height);
    RefPtr<Cairo::Context> canvas=Context::create(image);
    RenderSVG(canvas,*svgimg);
    ssvg::imageDestroy(svgimg);
    return image;
}
#endif

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
     NGLOG_DEBUG_IF(nullptr==closure.data,"name:%s data=%p size=%d  ext=%s",resname.c_str(),closure.data,closure.size,ext.c_str());
     RefPtr<ImageSurface>img;
     if(closure.data==nullptr)return img;
     if(ext=="svg"){
         //img=nano_loadSVG((const char*)closure.data,closure.size);
         img=loadSVG((const char*)closure.data,closure.size);
     }else if(ext=="png")
         img=ImageSurface::create_from_png(pak_read,&closure);
     else if(ext=="jpg"||ext=="jpeg"){
         img=ImageSurface::create_from_jpg(pak_read,&closure);
     }else if(ext=="bmp"){
         RefPtr<BasicBitmap>bmp(BasicBitmap::LoadBmpFromMemory(closure.data,closure.size,NULL));//
         NGLOG_VERBOSE("bmp size=%dx%d fmt=%d pitch=%d",bmp->Width(),bmp->Height(),bmp->Format(),bmp->Pitch());
         if(bmp->Format()!=BasicBitmap::A8R8G8B8){
            RefPtr<BasicBitmap>cvt(new BasicBitmap(bmp->Width(), bmp->Height(),BasicBitmap::A8R8G8B8));
            if (cvt)
                cvt->Convert(0, 0, (const BasicBitmap*)bmp, 0, 0, bmp->Width(), bmp->Height());
            bmp=cvt;
         }

         NGLOG_DUMP("BMP",bmp->Bits(),8);
         img=ImageSurface::create(bmp->Bits(),FORMAT_ARGB32,bmp->Width(),bmp->Height(),bmp->Pitch());
     }
     if(cache)
         images.insert(std::map<const std::string,RefPtr<ImageSurface> >::value_type(resname,img));
     NGLOG_VERBOSE("image size=%dx%d",img->get_width(),img->get_height());
     return img;
}

const std::string ResourceManager::getString(const std::string& id,const char*lan){
     string stringJson;
     Json::Reader reader;
     Json::Value root;
     if((lan!=nullptr)&&(strlen(lan)>=2)&&(osdlanguage!=lan)){
         std::string resname="strings-";
         resname.append(lan);
         resname.append(".json");
         osdlanguage=lan;
         PAKCLOSURE closure;
         closure.data=(unsigned char*)pak->getPAKEntryData(resname);
         closure.size=(size_t)pak->getPAKEntrySize(resname);
         const char*buf_start=(const char*)closure.data;
         reader.parse(buf_start,buf_start+closure.size,root);
         Json::Value::Members mem = root.getMemberNames();
 	 Json::Value::Members::const_iterator  it;
         for (auto k:mem){
             strings[k]=root[k].asString();
         }
     }
     NGLOG_DEBUG("id=%s string=%s",id.c_str(),strings[id].c_str());
     return strings[id];
}


ResourceManager::ResourceManager(const std::string&pakpath){
    std::unique_ptr<FilePAK>pp(new FilePAK());
    pak=std::move(pp);
    pak->readPAK(pakpath);
}

}//namespace

