#include<canvas.h>
#include<pixman.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftcache.h>
#include <freetype/ftstroke.h>
#include <freetype/ftbbox.h>
#include <fontmanager.h>

namespace nglui{

Canvas::Canvas(void*p){
    pixman=p;
    _color=0xFFFFFFFF;
    memset(&font,0,sizeof(LOGFONT));
    font.lfWidth=font.lfHeight=20;
    stroker=nullptr;
    FT_Stroker_New(FontManager::getInstance().getLibrary(),(FT_Stroker*)&stroker);
    FT_Stroker_Set((FT_Stroker)stroker,3*64,FT_STROKER_LINECAP_ROUND,FT_STROKER_LINEJOIN_ROUND,0);//FT_STROKER_LINEJOIN_MITER_FIXED);
}

void Canvas::select_font(unsigned int weight,int style,const char*family){
    font.lfFaceName[0]=0;
    if(family)strncpy(font.lfFaceName,family,LF_FACESIZE-1);
    font.lfWidth=font.lfHeight=weight;
}

void Canvas::fill_rectangle(int x,int y,unsigned int w,unsigned h){
    pixman_box32_t r={x,y,(int32_t)(x+w),(int32_t)(y+h)};
    pixman_color_t color={(uint16_t)(((_color>>16)&0xFF)<<8),(uint16_t)(((_color>>8)&0xFF)<<8),(uint16_t)((_color&0xFF)<<8),(uint16_t)(_color>>24<<8)};
    pixman_image_fill_boxes(PIXMAN_OP_SRC,(pixman_image_t*)pixman,&color,1,&r);
}

void Canvas::get_text_extents(const char *text,TextExtents*te){
    FT_Face face;
    FT_Glyph glyph;
    FT_BBox bbox;
    unsigned int gidx;
    te->x_bearing= te->y_bearing = 0.0;
    te->width    = te->height = 0.0;
    te->x_advance= te->y_advance = 0.0;
    FontManager::getInstance().getFace(&font,&face);
    while(*text++){
        gidx=FontManager::getInstance().getGraphIndex(&font,*text);
        FontManager::getInstance().getGlyph(&font,gidx,&glyph,FT_LOAD_BITMAP_METRICS_ONLY);//FT_LOAD_COMPUTE_METRICS);
       // printf("glyph advance=%d,%d\r\n",glyph->advance.x,glyph->advance.y);
        //FT_Glyph_Get_CBox(glyph,FT_GLYPH_BBOX_GRIDFIT, &bbox);
        //printf("%c BBox %d,%d-%d,%d\r\n",*text,bbox.xMin>>6,bbox.xMax>>6,bbox.yMin>>6,bbox.yMax>>6);
    }
}

void Canvas::get_font_extents(FontExtents*fe){
    FT_Face face;
    FontManager::getInstance().getFace(&font,&face);
    printf("units_per_EM=%d\r\n",face->units_per_EM);
    double scale = 64;//face->units_per_EM;
#if 1 
    fe->ascent  =  face->size->metrics.ascender/scale;//face->ascender / scale;
    fe->descent =  -face->size->metrics.descender/scale;//- face->descender / scale;
    fe->height  =  fe->ascent+fe->descent;//face->height / scale;
#else
    fe->ascent  =  face->ascender / scale;
    fe->descent =  - face->descender / scale;
    fe->height  =  face->height / scale;
#endif
    if (0){//!_cairo_ft_scaled_font_is_vertical (&scaled_font->base)) {
        fe->max_x_advance = face->max_advance_width / scale;
        fe->max_y_advance = 0;
    } else {
        fe->max_x_advance = 0;
        fe->max_y_advance = face->max_advance_height / scale;
    } 
}

void Canvas::rectangle(int x,int y,unsigned int w,unsigned h){
    fill_rectangle(x,y,w,1);
    fill_rectangle(x+w,y,1,h);
    fill_rectangle(x,y+h,w,1);
    fill_rectangle(x,y,1,h);
}

void Canvas::move_to(int x,int y){
    FT_Vector p={x<<6,y<<6};
    FT_Stroker_BeginSubPath((FT_Stroker)stroker,&p,0);
    //FT_Stroker_LineTo((FT_Stroker)stroker,&p); 
}

void Canvas::line_to(int x,int y){
    FT_Vector p={x<<6,y<<6};
    FT_Stroker_LineTo((FT_Stroker)stroker,&p);
}

void Canvas::fill(){
}

static void RasterCallback(const int y,const int count,
		const FT_Span * const spans,void * const user)
{
	/*PNode sptr = (PNode)user;
	while (sptr->next != NULL)
		sptr = sptr->next;
	int i;
	for (i = 0; i < count; ++i) {
		PNode new = calloc(sizeof(Node), 1);
		if (!new) {
			printf("failed to alloc new node\n");
			break;
		}
		new->next = NULL;
		new->node.x = spans[i].x;
		new->node.y = y;
		new->node.width = spans[i].len;
		new->node.coverage = spans[i].coverage;
		sptr->next = new;
		sptr = sptr->next;
	}*/
        if(spans)printf("x=%d len=%d coverage=%d\r\n",spans->x,spans->len,spans->coverage);
        else printf("span is null\r\n");
}

void Canvas::stroke(){
    FT_Library lib=FontManager::getInstance().getLibrary();
    FT_Outline outline;
    FT_Raster_Params  params;
    memset(&params, 0, sizeof(params));
    params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
    params.gray_spans = RasterCallback;

    FT_Face face;FontManager::getInstance().getFace(&font,&face);
    unsigned int gindex = FontManager::getInstance().getGraphIndex(face,'@');
    FT_Load_Glyph(face, gindex, FT_LOAD_NO_BITMAP);

    //FT_Outline_New(lib,0,0,&outline);
    outline=face->glyph->outline;
    //FT_Stroker_Export((FT_Stroker)stroker,&outline );
    FT_Bitmap bmp;
    bmp.rows=pixman_image_get_height((pixman_image_t*)pixman);
    bmp.width=pixman_image_get_width((pixman_image_t*)pixman);
    bmp.pitch=pixman_image_get_stride((pixman_image_t*)pixman);
    bmp.buffer=(unsigned char*)pixman_image_get_data((pixman_image_t*)pixman)-bmp.pitch*100;
    bmp.pixel_mode= FT_PIXEL_MODE_GRAY;//BGRA;
    FT_BBox box;
    FT_Outline_Get_BBox(&outline,&box);
    FT_Outline_Get_Bitmap(lib,&outline,&bmp);
    printf("pitch=%d\r\n",bmp.pitch); 
//    printf("Outline_render=%d box:%d,%d-%d,%d\r\n",FT_Outline_Render(lib,&outline,&params),
//        box.xMin,box.xMax,box.yMin,box.yMax);
//        box.xMin>>6,box.xMax>>6,box.yMin>>6,box.yMax>>6);
    FT_Outline_Done(lib,&outline);
} 

static void RenderGlyph(pixman_image_t* pmd,int x,int y,FTC_SBit sbit){
    pixman_trapezoid tz={0,0,sbit->width,sbit->height};
    if(sbit->pitch%4)return ;
    //printf("sbit size %dx%d pitch=%d\r\n",sbit->width,sbit->height,sbit->pitch);
    pixman_image_t* pms=pixman_image_create_bits(PIXMAN_a8,sbit->width,sbit->height,(unsigned int*)sbit->buffer,sbit->pitch);
    pixman_image_composite(PIXMAN_OP_OVER,pms,NULL,pmd,0,0,0,0,x,y,sbit->width,sbit->height);
}
void Canvas::draw_text(int x,int y,const char*text_utf8){
    const char*ch=text_utf8;
    unsigned int colors[256];
    unsigned int previous=0;
    unsigned int *pixels=pixman_image_get_data((pixman_image_t*)pixman);
    unsigned stride=pixman_image_get_stride((pixman_image_t*)pixman)/4;
    FT_Face face;
    TextExtents te;
    FontManager::getInstance().getFace(&font,&face);
    for(int i=0;i<256;i++){
         colors[i]=(((_color>>24)*i/255)<<24)|((((_color>>16)&0xFF)*i/255)<<16)|
                ((((_color>>8)&0xFF)*i/255)<<8)|((_color&0xFF)*i/255);
    }
    FTC_SBit sbit=NULL;
    for(;*ch;ch++){
         FT_Vector  delta={0,0};
         FT_UInt charIdx=FontManager::getInstance().getGraphIndex(face,*ch);
#if 0 
         FT_Load_Glyph(face, charIdx, FT_LOAD_RENDER);
         for(int iy=0;iy<face->glyph->bitmap.rows;iy++){
             unsigned char*src=face->glyph->bitmap.buffer+iy*face->glyph->bitmap.pitch;
             unsigned int *dst=pixels+(stride>>2)*(y+iy-(face->glyph->metrics.horiBearingY>>6)+(face->ascender>>6))+x;
             for(int ix=0;ix<face->glyph->bitmap.width;ix++,src++){
                 dst[ix]=colors[*src];
             }
         }
         FT_Get_Kerning( face, previous, charIdx,FT_KERNING_DEFAULT, &delta );
         x+=(face->glyph->metrics.horiAdvance>>6)+(delta.x>>6);
#else
         FontManager::getInstance().getCharBitmap(&font,charIdx,&sbit);
         //RenderGlyph((pixman_image_t*)pixman,x,y,sbit);
         unsigned char*src=sbit->buffer;
         unsigned int *dst=pixels+stride*(y-sbit->top+(face->ascender>>6))+x;//sbit->top is bearingY
         for(int iy=0;iy<sbit->height;iy++,src+=sbit->pitch,dst+=stride){
             unsigned char*ps=src;
             for(int ix=0;ix<sbit->width;ix++,ps++){
                 //dst[ix]=(*ps++<<24)|0xFFFFFF;
                 if(*ps!=0)dst[ix]=colors[*ps];
             }
         }
         if(FT_Get_Kerning( face, previous, charIdx,FT_KERNING_DEFAULT, &delta )==FT_Err_Ok)   x+=(delta.x>>6);
         x+=sbit->xadvance;
#endif
         previous=charIdx;
    }
    get_text_extents(text_utf8,&te);//move_to(100,100);line_to(600,400);line_to(100,100);stroke();
}

}//namespace

