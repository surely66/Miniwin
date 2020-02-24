#include<ngl_graph.h>
#include<ngl_os.h>
#include <gtest/gtest.h>
/*#include <pixman.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <canvas.h>
#include <fontmanager.h>
*/
#include <sys/time.h>

class GRAPH:public testing::Test{
   public :
   virtual void SetUp(){
     nglGraphInit();
   }
   virtual void TearDown(){
   }
   unsigned long long gettime(){
       struct timeval tv;
       gettimeofday(&tv,NULL);
       return tv.tv_sec*1000+tv.tv_usec/1000;
   }
   unsigned int getPixel(HANDLE surface,int x,int y){
       BYTE*buffer;
       UINT w,h,f,pitch;
       nglLockSurface(surface,(void**)&buffer,&pitch);
       nglGetSurfaceInfo(surface,&w,&h,(int*)&f);
       buffer+=pitch*y;
       switch(f){
       case GPF_ARGB4444:
       case GPF_ARGB1555:return *(USHORT*)(buffer+2*x);
       case GPF_ARGB:
       case GPF_ABGR:
       case GPF_RGB32:return *(UINT*)(buffer+4*x);
       }
   }
};

TEST_F(GRAPH,Graph_GetScreen){
   UINT w,h;
   nglGetScreenSize(&w,&h);
   ASSERT_TRUE(w>0&&h>0);
}

TEST_F(GRAPH,CreateSurface){
   HANDLE surface=0;
   UINT width,height;
   nglGetScreenSize(&width,&height);
   nglCreateSurface(&surface,width,height,0,1);
   NGLRect r={100,100,400,400};
   nglFillRect(surface,&r,0xFFFF00FF);
   nglFlip(surface);
   ASSERT_TRUE(surface!=0);
   sleep(20);
   nglDestroySurface(surface);
}

typedef struct {
   int format;
   unsigned int rgb[4];
}TSTPIXEL;

TEST_F(GRAPH,Format){
    //this case show four color block ,RED,GREEN,BLUE,WHITE.
    HANDLE surface;
    UINT width,height;
    nglGetScreenSize(&width,&height);
    TSTPIXEL fpixels[]={
       {GPF_ARGB,    {0xFFFF0000,0xFF00FF00,0xFF0000FF,0xFFFFFFFF}},
       {GPF_ABGR,    {0xFF0000FF,0xFF00FF00,0xFFFF0000,0xFFFFFFFF}},
       {GPF_RGB32,   {0xFF0FF000,0xFF00FF00,0xFF0000FF,0xFFFFFFFF}},
       {GPF_ARGB4444,{0xFF00,0xF0F0,0xF00F,0xFFFF}},
       {GPF_ARGB1555,{0xFC00,0xFFE0,0x801F,0x1FFF}}
    };

    for(int i=0;i<sizeof(fpixels)/sizeof(TSTPIXEL)-3;i++){
        nglCreateSurface(&surface,width,height,fpixels[i].format,1);
	NGLRect r={100,100,100,100};
	nglFillRect(surface,&r,0xFFFF0000);
	ASSERT_EQ(getPixel(surface,101,101),fpixels[i].rgb[0]);
	r.x+=110;
	nglFillRect(surface,&r,0xFF00FF00);
	ASSERT_EQ(getPixel(surface,221,101),fpixels[i].rgb[1]);
	r.x+=110;
	nglFillRect(surface,&r,0xFF0000FF);
	ASSERT_EQ(getPixel(surface,331,101),fpixels[i].rgb[2]);
	r.x+=110,
	nglFillRect(surface,&r,0xFFFFFFFF);
	ASSERT_EQ(getPixel(surface,441,101),fpixels[i].rgb[3]);

	nglFlip(surface);
	nglDestroySurface(surface);
	nglSleep(5000);
    }
}

TEST_F(GRAPH,Surface_Draw){
   HANDLE surface=0;
   UINT width,height;
   UINT*buffer;
   UINT pitch;
   nglGetScreenSize(&width,&height);
   nglCreateSurface(&surface,width,height,0,1);
   ASSERT_TRUE(surface!=0);
   nglLockSurface(surface,(void**)&buffer,&pitch);
   for(int i=0;i<width*400;i++)     buffer[i]=0xFFFFFFFF;
   nglUnlockSurface(surface);
   NGLRect r={100,100,400,400};
   nglFillRect(surface,&r,0xFFFF00FF);
   nglFlip(surface);
   sleep(20);
   nglDestroySurface(surface);

}

TEST_F(GRAPH,Blit){
   HANDLE hwsurface;
   HANDLE swsurface;
   unsigned int width,height;
   nglGetScreenSize(&width,&height);
   nglCreateSurface(&hwsurface,width,height,0,1);
   NGLRect r1={0,0,width,height};
   nglFillRect(hwsurface,&r1,0xFF00000);
   nglFlip(hwsurface);

   nglCreateSurface(&swsurface,800,600,0,0);
   NGLRect rf={0,0,800,600};
   nglFillRect(swsurface,&rf,0xFF444444);
   NGLRect r={100,50,640,480};
   nglFillRect(swsurface,&r,0x0);
   nglBlit(hwsurface,NULL,swsurface,NULL);
   for(int y=-800;y<=800;y+=200){
       for(int x=-1000;x<1000;x+=200){
	   NGLRect drect={x,y,0,0};
	   nglBlit(hwsurface,&drect,swsurface,NULL);
           nglFlip(hwsurface);
           nglSleep(200);
       }
   }
   nglDestroySurface(swsurface);
   nglDestroySurface(hwsurface);
   sleep(20);
}
#if 0
TEST_F(GRAPH,Pixman){
    HANDLE surface;
    UINT width,height;
    UINT*buffer;
    UINT pitch;
    nglGetScreenSize(&width,&height);
    NGLRect r={0,0,1280,720};
    printf("screen size=%dx%d\r\n",width,height);
    nglCreateSurface(&surface,width,height,0,1);
    for(int i=0;i<32;i++){
       nglFillRect(surface,&r,0xFFFFFFFF);
       nglLockSurface(surface,(void**)&buffer,&pitch);
       pixman_image_t*img=pixman_image_create_bits(PIXMAN_a8r8g8b8,width,height,buffer,pitch);
       pixman_color_t color={0xFFFF,0x0,0,0xFFFF};//uint16 only used high 8bits.
       pixman_rectangle16_t rect={200,50,800,600};
       pixman_image_fill_rectangles((pixman_op_t)i,img,&color,1,&rect);
       pixman_triangle_t trig;
       pixman_point_fixed_t*p=&trig.p1;
       for(int j=0;j<3;j++){p->x=pixman_int_to_fixed(rand()%300);p->y=pixman_int_to_fixed(rand()%200);p++;}
       pixman_add_triangles(img,100,100,1,&trig);
       usleep(1000);
       pixman_image_unref(img);
       nglUnlockSurface(surface);
       nglFlip(surface);
    }
    sleep(3);
    nglDestroySurface(surface);
}

TEST_F(GRAPH,freetype){
    HANDLE surface;
    UINT width,height;
    FT_Library	library;
    FT_Face		face;
    UINT*buffer;
    UINT pitch;
    nglGetScreenSize(&width,&height);
    nglCreateSurface(&surface,width,height,0,1);
    nglLockSurface(surface,(void**)&buffer,&pitch);
    
    FT_Init_FreeType(&library);
    FT_New_Face(library,"/usr/lib/fonts/droid_chn.ttf",0,&face);
    FT_Set_Char_Size(face, 128*128, 128*128, 200,200);
    FT_UInt charIdx=FT_Get_Char_Index(face,'@');
    FT_Load_Glyph(face, charIdx, FT_LOAD_RENDER);//FT_LOAD_DEFAULT);
    
    NGLRect r={0,0,1280,720};
    printf("screen size=%dx%d-%d\r\n",width,height,pitch);
    printf("graph[%d] size=%dx%dx pitch=%d grays=%d\r\n",charIdx,face->glyph->bitmap.width,face->glyph->bitmap.rows,
         face->glyph->bitmap.pitch,face->glyph->bitmap.num_grays);
    
    for(int y=0;y<face->glyph->bitmap.rows;y++){
         BYTE*src=face->glyph->bitmap.buffer+y*face->glyph->bitmap.pitch;
         UINT *dst=buffer+(pitch>>2)*(100+y);
         for(int x=0;x<face->glyph->bitmap.width;x++,src++){
             if(*src)dst[100+x]=0xFF000000|((*src)<<16);
         }
    }
    nglUnlockSurface(surface);
    nglFlip(surface);
    sleep(3);

    nglDestroySurface(surface);
}

TEST_F(GRAPH,canvas){
    HANDLE surface;
    UINT width,height;
    UINT*buffer;
    UINT pitch;
    uint64_t tmstart;
    nglGetScreenSize(&width,&height);
    nglCreateSurface(&surface,width,height,0,1);
    nglLockSurface(surface,(void**)&buffer,&pitch);
    pixman_image_t*img=pixman_image_create_bits(PIXMAN_a8r8g8b8,width,height,buffer,pitch);

    nglui::FontManager::getInstance().loadFonts("/usr/lib/fonts");    
    nglui::FontExtents fe;
    nglui::Canvas c(img);
    c.set_color(0xFFFFFFFF);
    c.fill_rectangle(50,50,1000,650);
    c.rectangle(200,50,300,50);
    c.select_font(64);
    c.get_font_extents(&fe);
    printf("ascent=%f descent=%f height=%f max_advance=%f,%f\r\n",fe.ascent,fe.descent,fe.height,fe.max_x_advance,fe.max_y_advance);
    c.draw_text(200,50,"Hello world!");
    c.set_color(0xFFFF0088);
    c.rectangle(100,200,1000,1);
    c.select_font(40);
    tmstart=gettime();
    c.draw_text(100,200,"The quick brown fox jumps over a lazy dog");
    printf("1used time=%lld\r\n",gettime()-tmstart);
    
    tmstart=gettime();
    c.draw_text(100,200,"The quick brown fox jumps over a lazy dog");
    printf("2used time=%lld\r\n",gettime()-tmstart);
    pixman_image_ref(img);
    nglUnlockSurface(surface);
    nglFlip(surface);
    sleep(2);

    nglDestroySurface(surface);
}
#endif
