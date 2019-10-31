#include <windows.h>
#include <appmenus.h>
#include <dvbepg.h>
#include <ngl_types.h>
#include <ngl_log.h>
#include <ngl_ir.h>
#include <ngl_mediaplayer.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#define CHANNEL_LIST_ITEM_HEIGHT 40
#define IDC_CHANNELS 100

NGL_MODULE(MULTIMEDIA)
#define W_ID   100
#define W_PATH 600
#define W_SIZE 200
#define W_DATE 250

namespace ntvplus{

class MediaItem:public ListView::ListItem{
public:
public:
    int id;
    std::string path;
    size_t size;
    bool isdir;
    std::string datetime;
public:
    MediaItem(int id_,const std::string&fname):ListView::ListItem(std::string()){
        struct stat st;
        id=id_;
        size_t pos=fname.rfind("/");
        std::string name=fname.substr(pos+1);
        setText(name);
        path=fname.substr(0,pos);
        stat(fname.c_str(),&st);
        size=st.st_size;
        isdir=S_ISDIR(st.st_mode);
        struct tm *ftm=localtime(&st.st_mtime);
        char buf[64];
        sprintf(buf,"%02d/%02d/%d %02d:%02d",ftm->tm_mon+1,ftm->tm_mday,ftm->tm_year+1900,ftm->tm_hour,ftm->tm_min);
        datetime=buf;
    }
    virtual void onGetSize(AbsListView&lv,int* w,int* h)override{
        if(h)*h=CHANNEL_LIST_ITEM_HEIGHT;
    }
};

static void MediaPainter(AbsListView&lv,const ListView::ListItem&itm,int state,GraphContext&canvas){
    MediaItem& md=(MediaItem&)itm;
    char buf[32];
    canvas.set_color(state?0xFF00FF00:lv.getBgColor());
    canvas.draw_rect(itm.rect);
     
    RECT r=itm.rect;
    canvas.set_color(lv.getFgColor());
    sprintf(buf,"%3d",md.id);
    canvas.draw_text(r,buf,DT_LEFT|DT_VCENTER);//id
    r.offset(W_ID,0);//id width;
    canvas.draw_text(r,md.getText(),DT_LEFT|DT_VCENTER);//media filename;
    
    r.offset(W_PATH,0);//program width
    if(md.size>1024*1024){
         sprintf(buf,"%.2fM",((float)md.size)/(1024*1024));
    }else if(md.size>1024){
         sprintf(buf,"%.2fK",((float)md.size)/1024);
    }else{
         sprintf(buf,"%dB",md.size);
    }
    canvas.draw_text(r,buf,DT_LEFT|DT_VCENTER);//media size
    r.offset(W_SIZE,0);
    canvas.draw_text(r,md.datetime,DT_LEFT|DT_VCENTER);//datetime
}

class MediaWindow:public NTVWindow{
protected:
   ToolBar*mdtype;
   ToolBar*header;
   ListView*mdlist;
   std::string media_path;
   bool sort_revert;
   DWORD player;
public:
const static std::string FILTERS[];
public:
   MediaWindow(int x,int y,int w,int h);
   int loadMedia(const std::string&path,const std::string&filter);
   virtual bool onKeyRelease(KeyEvent&k)override;
};
const std::string MediaWindow::FILTERS[]={
      "mp4;mkv;avi;mpg;mpeg;ts",
      "mp3;wma;wav",
      "jpg;jpeg;png;bmp;ps;pdf",
      ""//empty string for all files
 };
 
MediaWindow::MediaWindow(int x,int y,int w,int h):NTVWindow(x,y,w,h){
    player=0;
    sort_revert=false;
    initContent(NWS_TITLE|NWS_TOOLTIPS);
    mdtype=CreateNTVToolBar(1200,30);

    mdtype->setPos(40,70);
    mdtype->addButton("Movie",-1,120);
    mdtype->addButton("Music",-1,120);
    mdtype->addButton("Photo",-1,120);
    mdtype->addButton("Folders",-1,120);
    mdtype->setIndex(0);
    addChildView(mdtype);

    header=new ToolBar(1200,30);
    header->setBgColor(0xFF000000);
    header->setPos(40,100);
    header->addButton("NO.",-1,W_ID);
    header->addButton("Program",-1,W_PATH);
    header->addButton("Size",-1,W_SIZE);
    header->addButton("Date/Time",-1,W_DATE);
    addChildView(header);
       
    mdlist=new ListView(1200,520);
    mdlist->setPos(40,130);
    mdlist->setFlag(Attr::ATTR_SCROLL_VERT);
    mdlist->setBgColor(getBgColor());
    mdlist->setFgColor(getFgColor());
    mdlist->setItemPainter(MediaPainter);
    addChildView(mdlist);
    mdlist->setClickListener([&](View&lv){
        int index=mdlist->getIndex();
        MediaItem*itm=(MediaItem*)mdlist->getItem(index);
        if(itm->isdir)loadMedia(itm->path+"/"+itm->getText(),FILTERS[index]);
    });
}

bool MediaWindow::onKeyRelease(KeyEvent&k){
    std::string media_file;
    MediaItem*itm=(MediaItem*)mdlist->getItem(mdlist->getIndex());
    if(itm)media_file=itm->path+"/"+itm->getText();

    switch(k.getKeyCode()){
    case NGL_KEY_LEFT:
    case NGL_KEY_RIGHT:{
            bool rc=mdtype->onKeyRelease(k);
            int idx=mdtype->getIndex();
            loadMedia(media_path,FILTERS[idx]);
            return rc;
         }break;
    case NGL_KEY_ENTER:{
           /*if(player){
                nglMPStop(player);
                nglMPClose(player);
           }*/
           CreatePlayerCtrlWindow(media_file);
           //player=nglMPOpen(media_file.c_str());
           //nglMPPlay(player);
           return true;
        }
    case KEY_RED:
         mdlist->sort([](const ListView::ListItem&a,const ListView::ListItem&b)->int{
                            return strcmp(a.getText().c_str(),b.getText().c_str())>0;
                       },sort_revert); 
         sort_revert=!sort_revert;
         for(int i=0;i<mdlist->getItemCount();i++){
              MediaItem*itm=(MediaItem*)mdlist->getItem(i);
              itm->id=i;
         }mdlist->invalidate(nullptr);
         break;
    case KEY_YELLOW:remove(media_file.c_str());break;
    case KEY_GREEN: 
           rename(media_file.c_str(),media_file.c_str());
         break;
    default: return NTVWindow::onKeyRelease(k);
    }
}

static std::vector<std::string> split(const std::string& s,const std::string& delim){
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len){
        int find_pos = s.find(delim, pos);
        if (find_pos < 0){
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

int MediaWindow::loadMedia(const std::string&path,const std::string&filter){
    int count=0;
    DIR *dir=opendir(path.c_str());
    struct dirent *ent;
    std::vector<std::string>exts=split(filter,";");
    mdlist->clearAllItems();
    NGLOG_DEBUG("%s :%s",path.c_str(),filter.c_str());
    if(dir==nullptr)return 0;
    while(ent=readdir(dir)){
        bool match=exts.size()==0;
        std::string fname=path+"/"+ent->d_name;
        std::string ext=fname.substr(fname.rfind(".")+1);
        for(auto e:exts){
             if(strcasecmp(e.c_str(),ext.c_str())==0){match=true;break;}
        }
        if((match&&(ent->d_type==DT_REG) )||(ent->d_type==DT_DIR))
             mdlist->addItem(new MediaItem(count++,fname));
    }
    media_path=path;
    closedir(dir);
    return count;
}

Window*CreateMultiMedia(){
    MediaWindow*w=new MediaWindow(0,0,1280,720);
    w->setText("Media");
    w->addTipInfo("help_icon_4arrow.png","Navigation",50,160);
    w->addTipInfo("help_icon_ok.png","Select",-1,160);
    w->addTipInfo("help_icon_back.png","Back",-1,160);
    w->addTipInfo("help_icon_exit.png","Exit",-1,260);
    w->addTipInfo("help_icon_red.png","A->Z",-1,160);
    w->addTipInfo("help_icon_green.png","Rename",-1,160);
    w->addTipInfo("help_icon_yellow.png","Delete",-1,160);
    w->loadMedia("/mnt/usb/sda1",MediaWindow::FILTERS[0]);
    w->show();
    return w;
}
}//namespace
