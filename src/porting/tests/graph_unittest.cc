#include<ngl_graph.h>
#include<ngl_os.h>
#include <gtest/gtest.h>
#include <pixman.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <canvas.h>
#include <sys/time.h>
#include <fontmanager.h>

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
};

TEST_F(GRAPH,Graph_GetScreen){
   UINT w,h;
   nglGetScreenSize(&w,&h);
   ASSERT_TRUE(w>0&&h>0);
}

TEST_F(GRAPH,CreateSurface){
   DWORD surface=0;
   UINT width,height;
   nglGetScreenSize(&width,&height);
   nglCreateSurface(&surface,width,height,0,1);
   NGLRect r={100,100,400,400};
   nglFillRect(surface,&r,0xFFFF0000);
   nglFlip(surface);
   ASSERT_TRUE(surface!=0);
   sleep(2);
   nglDestroySurface(surface);
}

TEST_F(GRAPH,Surface_Draw){
   DWORD surface=0;
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
   sleep(3);
   nglDestroySurface(surface);

}

TEST_F(GRAPH,Blit){
   DWORD hwsurface=0,swsurface;
   nglCreateSurface(&hwsurface,1280,720,0,1);
   NGLRect r1={0,0,1280,720};
   nglFillRect(hwsurface,&r1,0x00000);
   nglFlip(hwsurface);

   nglCreateSurface(&swsurface,800,600,0,0);
   NGLRect rf={0,0,800,600};
   nglFillRect(swsurface,&rf,0xFF444444);
   NGLRect r={100,50,640,480};
   nglFillRect(swsurface,&r,0x0);
   for(int i=-1;i<10;i++){
       for(int j=0;j<13;j++){
           nglBlit(hwsurface,swsurface,NULL,NULL);
           nglFlip(hwsurface);
           nglSleep(200);
       }
   }
   nglDestroySurface(swsurface);
   nglDestroySurface(hwsurface);
}

TEST_F(GRAPH,Pixman){
    DWORD surface;
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
    DWORD surface;
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
    DWORD surface;
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
