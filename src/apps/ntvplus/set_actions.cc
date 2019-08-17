#include<appmenus.h>
/*typedef std::function<void(View&v,int value)>SettingChangeListener;
typedef std::function<Window*(int)>SubMenuCreateListener;
typedef std::function<void(Window*,int id)>SettingDataLoadingListener;
typedef struct{
    SettingChangeListener onSettingChange;
    SubMenuCreateListener onSubMenuCreate;
    SettingDataLoadingListener onLoadData;
}SettingConfig;*/
extern int getNetworkInfo(const char*sfname,char*sipaddr,char*smask,char*sgetway,char*mac);
extern int setNetworkInfo(const char*sfname,char*sipaddr,char*smask,char*sgateway,char*smac);
extern int getNetworkInterface(std::vector<std::string>&nets);
namespace ntvplus{

#define ID_FIRST_EDITABLE_ID 100
static void time_load(Window*w){
    char buf[32];
    Selector*s=(Selector*)w->findViewById(ID_FIRST_EDITABLE_ID);
    for(int i=-12;i<=12;i++){
        sprintf(buf,"%02d:00",i);
        s->addItem(new Selector::ListItem(buf,i));
    }
    s=(Selector*)w->findViewById(ID_FIRST_EDITABLE_ID+1);

    time_t tnow=time(NULL);
    struct tm *tmnow=localtime(&tnow);
    printf("zone offset=%d\r\n",(mktime(tmnow)-tnow)/3600);
    EditBox*edt=(EditBox*)w->findViewById(ID_FIRST_EDITABLE_ID+2);
    edt->setEditMode(1);
    sprintf(buf,"%02d:%02d",tmnow->tm_hour,tmnow->tm_min);
    edt->setText(buf);
}
static void time_changed(View&v,int value){
    switch(v.getId()){
    case ID_FIRST_EDITABLE_ID:{
             Selector&s=(Selector&)v;
             char zone[8];
             sprintf(zone,"UTC%s%d",(value>=0?"+":""),value);
             setenv("TZ",zone,1);tzset();
             printf("timezone setto %s\r\n",zone);
        }break;
    default:break;
    }
}

static void network_load(Window*w){
    std::vector<std::string>nets;
    int cnt=getNetworkInterface(nets);
    Selector*s=(Selector*)w->findViewById(ID_FIRST_EDITABLE_ID);
    for(auto name:nets){
        s->addItem(new Selector::ListItem(name));
    }
    s->setItemSelectListener([](AbsListView&lv,int index){
          char ip[32],mask[32],mac[32],gateway[32];
          std::string ifname=lv.getItem(index)->getText();
          getNetworkInfo(ifname.c_str(),ip,mask,gateway,mac);
          Window*w=(Window*)lv.getParent();
          EditBox*e=(EditBox*)w->findViewById(ID_FIRST_EDITABLE_ID+1);
          e->setText(ip);
          e=(EditBox*)w->findViewById(ID_FIRST_EDITABLE_ID+2);
          e->setText(mask);
          e=(EditBox*)w->findViewById(ID_FIRST_EDITABLE_ID+3);
          e->setText(gateway);    
    });
    s->setIndex(0);
}

static void network_changed(View&v,int value){
    printf("=====network_changed %d\r\n",v.getId());
}

typedef struct{
   int id;
   SettingDataLoadingListener onLoadData;
   SettingChangeListener onSettingChange; 
}UI_ACTIONS;

static UI_ACTIONS setting_actions[]={
   {1,time_load,time_changed},
   {11,network_load,network_changed}
};

bool NTVSettingLoadData(Window*w,int id){
   for(int i=0;i<sizeof(setting_actions)/sizeof(UI_ACTIONS);i++){
       if(setting_actions[i].id==id){
           if(setting_actions[i].onLoadData!=nullptr)
               setting_actions[i].onLoadData(w);
           return true;
       }
   }
   return false;
}

void NTVSettingChanged(int winid,View&v,int value){
   printf("Editable UI[%d].ctrls[%d] changed to %d\r\n",winid,v.getId(),value);
   for(int i=0;i<sizeof(setting_actions)/sizeof(UI_ACTIONS);i++){
       if( (setting_actions[i].id==winid) && (setting_actions[i].onSettingChange!=nullptr) ){
            setting_actions[i].onSettingChange(v,value);
            return ;
       }
   }
}

Window*NTVCreateCustomSettingWindow(int id){
    switch(id){
    case -1: return CreatePVRWindow();
    case -2: return CreateMultiMedia();
    case -50:return CreateChannelSearch();
    case -51:return CreateChannelSearch(1);
    }
}

}//namespace 