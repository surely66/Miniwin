#include <gtest/gtest.h>
#include <ngl_types.h>
#include <ngl_graph.h>
#include <graph_context.h>
#include <ngl_os.h>
#include <ngl_timer.h>
#include <cairomm/basicbitmap.h>
#include <resourcemanager.h>
class CONTEXT:public testing::Test{
protected:
   NGL_RunTime ts,te;
public :
   nglui::ResourceManager *rm;
   virtual void SetUp(){
     rm=new nglui::ResourceManager("ntvplus.pak");
   }
   virtual void TearDown(){
   }
   void tmstart(){
      nglGetRunTime(&ts);
   }
   void tmend(const char*txt){
      nglGetRunTime(&te);
      printf("%s:used time %dms\r\n",txt,te.uiMilliSec+te.uiMicroSec/1000-ts.uiMilliSec-ts.uiMicroSec/1000);
   }
};

TEST_F(CONTEXT,SURFACE_CREATE_1){
   RefPtr<GraphContext>ctx1(GraphDevice::getInstance()->createContext(800,600));
   ctx1->set_color(0,0,255);
   ctx1->rectangle(0,0,800,600);
   ctx1->fill();
   for(int j=0;j<10;j++){
      for(int i=0;i<10;i++){
         const char *txt[]={"Beijing","Sigapo","ShangHai","Shenzhen",
            "The quick brown fox jumps over a lazy dog",
            "Innovation in China","中国智造，惠及全球 0123456789"};
         ctx1->set_font_size(i==j?40:28);
         ctx1->save();
         if(i==j){
             RECT rc={400,i*40,400,40};
             ctx1->set_color(255,0,0);
             ctx1->draw_rect(0,i*40,800,40);
             ctx1->set_color(255,255,255);
             ctx1->draw_text(10,i*40,txt[i%(sizeof(txt)/sizeof(char*))]);
         }else{
             ctx1->rectangle(0,i*40,800,40);
             ctx1->fill();
             ctx1->set_color(255,255,255);
             ctx1->draw_text(10,i*40,txt[i%(sizeof(txt)/sizeof(char*))]);
         }
         ctx1->restore();
      }ctx1->flip();
   }
   ctx1->dump2png("test3.png");
   nglSleep(2000);
}

TEST_F(CONTEXT,TEXT_ALIGNMENT){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(1080,720));
    const char*horz[]={"LEFT","CENTER","RIGHT"};
    const char*vert[]={"TOP","VCENTER","BOTTOM"};
    char alignment[64];
    RECT rect={100,100,800,120};
    ctx->set_font_size(40);
    for(int h=0;h<=2;h++){
        for(int v=0;v<=2;v++){
           ctx->set_color(0xFFFFFFFF);
           ctx->rectangle(0,0,1080,720);
           ctx->fill();
           sprintf(alignment,"%s_%s",horz[h],vert[v]);
           printf("test %s\r\n",alignment);
           ctx->set_color(0xFFFF0000);
           ctx->draw_text(20,20,alignment);
           ctx->draw_rect(rect);
           ctx->set_color(0xFF00FF00);
           ctx->draw_text(rect,"The quick brown fox jump sover the lazy dog.",(v<<4)|h);
           ctx->flip();
           strcat(alignment,".png");
           ctx->dump2png(alignment);
           nglSleep(1000);
        }nglSleep(1000); 
    }
    nglSleep(2000);
}
TEST_F(CONTEXT,Bitmap){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    RefPtr<BasicBitmap>bmp(BasicBitmap::LoadBmp("Im_win_bodyright_l.bmp",BasicBitmap::A8R8G8B8));
    RefPtr<ImageSurface>img=ImageSurface::create(bmp->Bits(),FORMAT_ARGB32,bmp->Width(),bmp->Height(),bmp->Pitch());
    RECT rect={0,0,800,600};
    for(int i=0;i<10;i++){
      ctx->set_color(0xFF000000|(i*20<<16));
      ctx->draw_rect(rect);
      ctx->draw_image(img,10,10);//,&rect,NULL,ST_FITXY);//ST_CENTER_INSIDE);
      ctx->flip();
      nglSleep(500);
    }
    ctx->flip();
    nglSleep(2000);
}
TEST_F(CONTEXT,Image_PNG){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    tmstart();
    RefPtr<ImageSurface>img=ImageSurface::create_from_png("light.png");
    tmend("decodepng");
    RECT rect={0,0,800,600};
    ctx->draw_rect(rect);
    tmstart();
    ctx->draw_image(img,&rect,NULL,ST_CENTER_INSIDE);
    tmend("drawimage");
    ctx->flip();
    //ctx->dump2png("test2.png");
    nglSleep(2500);
}

TEST_F(CONTEXT,Image_PNG_1){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    tmstart();
    RefPtr<ImageSurface>img=ImageSurface::create_from_png("a3.png");
    tmend("decodepng");
    RECT rect={0,0,800,600};
    for(int i=0;i<10;i++){
    ctx->set_color(0xFF000000|i*20|(i*20<<8));
    ctx->draw_rect(rect);
    tmstart();
    ctx->draw_image(img,10,10);//,&rect,NULL,ST_FITXY);//ST_CENTER_INSIDE);
    tmend("drawimage");
    ctx->flip();
    nglSleep(500);
    }
    //ctx->dump2png("test2.png");
    nglSleep(2500);
}

TEST_F(CONTEXT,Image_JPG){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    tmstart();
    RefPtr<ImageSurface>imgj=ImageSurface::create_from_jpg("landscape.jpg");
    tmend("decodejpg");
    RECT rect={0,0,800,600};
    ctx->draw_rect(rect);
    tmstart();
    ctx->draw_image(imgj,&rect,NULL,ST_CENTER_INSIDE);
    tmend("drawimage");
    ctx->flip();
    nglSleep(2500);
}

TEST_F(CONTEXT,Resource_Image){
   RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    tmstart();
    RefPtr<ImageSurface>img=rm->loadImage("hs-400.png");//"hd_mainmenu.bmp");
    tmend("decodejpg");
    RECT rect={0,0,800,600};
    ctx->draw_rect(rect);
    tmstart();
    ctx->draw_image(img,&rect,NULL,ST_CENTER_INSIDE);
    tmend("drawimage");
    ctx->flip();
    nglSleep(2500);
}

TEST_F(CONTEXT,subcanvas){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    RefPtr<GraphContext>ctx1(new GraphContext(*ctx,100,100,300,200));
    RefPtr<GraphContext>ctx2(new GraphContext(*ctx1,100,100,100,100));
    RefPtr<GraphContext>ctx3(new GraphContext(*ctx2,50,50,50,50));
    ctx->set_color(0xFFFFFFFF);
    ctx->draw_rect(0,0,800,600);
    ctx1->set_color(0xFF00FF00);
    ctx1->draw_rect(0,0,300,200);
    ctx2->set_color(0xFF0000FF);
    ctx2->draw_rect(0,0,100,100);
    ctx3->set_color(0xFFFF0000);
    ctx3->draw_rect(0,0,50,50);
    ctx->flip();
    nglSleep(2500);
}

TEST_F(CONTEXT,Translate){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    ctx->set_color(0xFFFFFFFF);
    ctx->draw_rect(0,0,800,600);
    ctx->set_color(0xFFFF0000);
    ctx->draw_rect(0,0,200,200);

    ctx->translate(100,100);
    ctx->set_color(0xFF00FF00);
    ctx->draw_rect(0,0,100,100);

    ctx->translate(-50,-50);
    ctx->set_color(0xFF0000FF);
    ctx->draw_rect(0,0,100,100);
    
    ctx->translate(-50,-50);
    ctx->set_color(0xFFFFFF00);
    ctx->rectangle(0,0,100,100);
    ctx->stroke();

    ctx->flip();
    nglSleep(2500);
}

TEST_F(CONTEXT,Clip){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    RefPtr<ImageSurface>img=ImageSurface::create_from_png("light.png");
    ctx->set_color(0xFFFFFFFF);
    ctx->draw_rect(0,0,800,600);
    ctx->arc(400,300,100,0,3.14159*2);
    ctx->clip();
    RECT rect={250,150,200,200};
    ctx->draw_image(img,&rect,NULL,ST_CENTER_INSIDE);
    ctx->flip();
    nglSleep(1000);
}

TEST_F(CONTEXT,Mask){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
    RefPtr<ImageSurface>img=ImageSurface::create_from_png("light.png");
    ctx->set_color(0xFFFFFFFF);
    ctx->draw_rect(0,0,800,600);
    
    ctx->flip();
    nglSleep(1000);
}

TEST_F(CONTEXT,Pattern_Line){
   RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(800,600));
   int i, j; 
   RefPtr<RadialGradient>radpat(RadialGradient::create(200, 150, 80, 400, 300, 400));
   RefPtr<LinearGradient>linpat(LinearGradient::create(200, 210, 600, 390));
   ctx->set_color(0xffffffff);
   ctx->draw_rect(0,0,800,600);
   radpat->add_color_stop_rgb ( 0, 1.0, 0.8, 0.8); 
   radpat->add_color_stop_rgb ( 1, 0.9, 0.0, 0.0); 
   for (i=1; i<10; i++) 
        for (j=1; j<10; j++) 
            ctx->rectangle ( i*50, j*50,48 ,48); 
   ctx->set_source ( radpat); 
   ctx->fill (); 
   linpat->add_color_stop_rgba ( .0, 1, 1, 1, 0); 
   linpat->add_color_stop_rgba ( .2, 0, 1, 0, 0.5); 
   linpat->add_color_stop_rgba ( .4, 1, 1, 1, 0); 
   linpat->add_color_stop_rgba ( .6, 0, 0, 1, 0.5); 
   linpat->add_color_stop_rgba ( .8, 1, 1, 1, 0); 
   ctx->rectangle ( 0, 0, 800, 600); 
   ctx->set_source(linpat); 
   ctx->fill ();
   ctx->flip();
   nglSleep(2000); 
}
TEST_F(CONTEXT,Pattern_Radio){
   RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(1200,600));
   ctx->set_color(0,0,0);
   ctx->draw_rect(0,0,1200,600);
   RefPtr<RadialGradient>radpat(RadialGradient::create(200, 200, 10, 200, 200, 150));
   radpat->add_color_stop_rgb ( .0, 1., 1., 1.);
   radpat->add_color_stop_rgb ( 1., 1., .0,.0);
   ctx->set_source ( radpat);
   ctx->arc(200,200,150,.0,3.1415*2);
   ctx->fill ();
   
   ctx->flip();
   nglSleep(2000);
}

TEST_F(CONTEXT,Font){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(1280,720));
    const char *texts[]={
        "The quick brown fox jumps over a lazy dog",
        "Innovation in China 中国智造，惠及全球 0123456789" 
    };
    ctx->set_color(0xffffffff);
    ctx->draw_rect(0,0,1280,720);
    auto lg=LinearGradient::create(10,480,1100,720);
    lg->add_color_stop_rgba(.0,1.0,0,0,0.5);
    lg->add_color_stop_rgba(.5,.0,1.,.0,1.);
    lg->add_color_stop_rgba(1.,.0,0,.1,0.5);
    for(int i=0,y=10;i<10;i++){
       std::string txt=texts[i%(sizeof(texts)/sizeof(char*))];
       int font_size=10+i*5;
       //cant use this select_font_face use defaut font pls.
       //ctx->select_font_face(faces[i],FONT_SLANT_NORMAL, FONT_WEIGHT_BOLD);
       ctx->set_font_size(font_size);
       ctx->move_to(0,y);y+=font_size+10;
       if(i%4==0)ctx->set_color(0,0,0);
       else ctx->set_source(lg);
       if(i%2==0){
           ctx->show_text(txt);
       }else{
           ctx->text_path(txt);ctx->stroke();
       }
    }
    
    ctx->set_font_size(150);
    for(int i=0;i<20;i++){
       auto lg=LinearGradient::create(20*i,480,300+50*i,720);
       lg->add_color_stop_rgba(.0,1.0,0,0,0.5);
       lg->add_color_stop_rgba(.5,.0,1.,.0,1.);
       lg->add_color_stop_rgba(1.,.0,0,.1,0.5);
       ctx->set_source(lg);
       ctx->move_to(0,600);
       nglSleep(10);
       ctx->show_text("中国智造,惠及全球");
       ctx->flip();
    }
    nglSleep(1000);
}
TEST_F(CONTEXT,ALPHA){
     RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(1280,720));
     ctx->set_color(0xFFFFFFFF);
     ctx->draw_rect(0,0,1280,720);
     ctx->set_color(0x80FF0000);
     ctx->draw_rect(200,200,480,320);
     ctx->dump2png("alpha.png");
}
TEST_F(CONTEXT,HOLE){
     RefPtr<GraphContext>ctx(GraphDevice::getInstance()->createContext(1280,720));
     ctx->set_color(0xFFFFFFFF);
     ctx->draw_rect(0,0,1280,720);
     ctx->set_color(0);
     ctx->draw_rect(200,200,480,320);
     ctx->dump2png("hole.png");
}
TEST_F(CONTEXT,HOLE2){
    RefPtr<GraphContext>ctx(GraphDevice::getInstance()->getPrimaryContext());
     ctx->set_color(0xFFFFFFFF);
     ctx->draw_rect(0,0,1280,720);
     ctx->set_source_rgba(0,0,0,0);
     ctx->draw_rect(200,200,480,320);
     ctx->dump2png("hole2.png");
}
TEST_F(CONTEXT,HOLE3){
     RefPtr<ImageSurface>img=ImageSurface::create(FORMAT_ARGB32,1280,720);
     RefPtr<Context>ctx=Context::create(img);
     ctx->set_source_rgb(1,0.5,1);
     ctx->rectangle(0,0,1280,720);
     ctx->fill();
     ctx->set_source_rgba(0,1,0,0.1);
     ctx->rectangle(200,200,480,320);
     ctx->fill();
     img->write_to_png("hole3.png");
}
