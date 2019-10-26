#include <favorite.h>
#include <si_table.h>
#include <tvdata.h>
#include <string>
#include <vector>
#include <json/json.h>
#include <iostream>
#include <fstream>

NGL_MODULE(FAVGROUP)

typedef struct FavGroup{
   int id;
   std::string grpname;
   std::vector<SERVICELOCATOR>services;
public:
   FavGroup(int id_,const std::string&name){
      id=id_;
      grpname=name;
   }
}FAVGROUP; 

static std::vector<FAVGROUP>favgroups;

int FavInit(const char*favpath){
   NGLOG_DEBUG("sizeof SERVICELOCATOR=%d",sizeof(SERVICELOCATOR));
   NGLOG_DEBUG("todo load favorites' data from storage");
   FavAddGroupWithID(FAV_GROUP_ALL,"ALL"); 
   FavAddGroupWithID(FAV_GROUP_AV,"Audio&Video"); 
   FavAddGroupWithID(FAV_GROUP_VIDEO,"Video"); 
   FavAddGroupWithID(FAV_GROUP_AUDIO,"Audio"); 
   DtvCreateSystemGroups();
   DtvCreateGroupByBAT();
   FavLoadData(favpath);
}

int FavSaveData(const char*fname){
    Json::Value root;
    int idx=0;
    for(auto g:favgroups){
        if(g.id&(GROUP_SYSTEM|GROUP_BAT))continue;
        root[idx]["id"]=g.id;
        root[idx]["name"]=g.grpname;
        for(int j=0;j<g.services.size();j++){
             SERVICELOCATOR& s=g.services[j];
             Json::Value js;
             js["netid"]=s.netid;
             js["tsid"]=s.tsid;
             js["sid"]=s.sid;
             root["services"][j]=js;
        }idx++;
    }
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream fout(fname);
    writer->write(root,&fout);
    return idx;
}

int FavLoadData(const char*fname){
    Json::CharReaderBuilder builder;
    std::ifstream fin(fname);
    Json::Value root;
    Json::String errs;
    bool rc=Json::parseFromStream(builder,fin, &root, &errs);
    NGLOG_DEBUG_IF(!rc,"json.parse=%d %s",rc,errs.c_str());
    if(!root.isArray()){
        NGLOG_ERROR("%s not found or format is error",fname);
        return NGL_ERROR;
    }
    for(int i=0;i<root.size();i++){
        Json::Value g=root[i];
        UINT favid=g["id"].asInt();
        if( favid&(GROUP_SYSTEM|GROUP_BAT) ){
             NGLOG_DEBUG("invalid favid %x",favid);
             continue;
        }
        FavAddGroupWithID(favid,g["name"].asCString());
        for(int j=0;j<g["services"].size();j++){
             SERVICELOCATOR l;
             Json::Value v=g["services"][j];
             l.netid=v["netid"].asInt();
             l.tsid=v["tsid"].asInt();
             l.sid=v["sid"].asInt();
             FavAddService(favid,&l);
        }
    }
    NGLOG_DEBUG("load %d favorites",root.size());
    return root.size();
}

int FavGetGroupCount(){
    return favgroups.size();
}

int FavAddGroup(const char*name){
    bool name_exist=false;
    int favid=0;
    for(auto g:favgroups){
        if(g.grpname.compare(name)==0){
            name_exist=true;
        }
        if( (g.id&(GROUP_SYSTEM|GROUP_BAT))==0){
           favid++;
        }
    } 
    if(name_exist)return NGL_ERROR;
    FAVGROUP grp(favid,name);
    favgroups.push_back(grp);
    return favid;
}

int FavAddGroupWithID(UINT id,const char*name){//only used for module owner
    FAVGROUP grp(id,name);
    for(int i;i<favgroups.size();i++){
         if(favgroups[i].id==id)
            return NGL_ERROR;
    }
    favgroups.push_back(grp);
    return 0;
}

static FAVGROUP*FindFavGroupById(UINT favid){
    for(int i=0;i<favgroups.size();i++){
         if(favgroups[i].id==favid)
            return &favgroups[i];
    }
    return NULL;
}

int FavRemoveGroup(UINT favid){
    for(auto g=favgroups.begin();g!=favgroups.end();g++){
        if((*g).id==favid){
            favgroups.erase(g);
            return 0;
        }
    }
    return 0;
}

int FavGetGroupInfo(int idx,UINT *favid,char*name){
    *favid=favgroups[idx].id;
    strcpy(name,favgroups[idx].grpname.c_str());
    return  0;
}

int FavGetGroupName(UINT favid,char*name){
    FAVGROUP*grp=(FAVGROUP*)FindFavGroupById(favid);
    if(grp)strcpy(name,grp->grpname.c_str());
    return grp?NGL_OK:NGL_ERROR;
}

UINT FavGetServiceCount(UINT favid){
    FAVGROUP*grp=(FAVGROUP*)FindFavGroupById(favid);
    return grp==NULL?0:grp->services.size();
}

int FavGetServices(UINT favid,SERVICELOCATOR*svcs,int maxitem){
    FAVGROUP*grp=(FAVGROUP*)FindFavGroupById(favid);
    for(int i=0;i<grp->services.size()&&i<maxitem;i++){
         svcs[i]=grp->services[i];
    }
    return 0;
}

int FavGetService(UINT favid,SERVICELOCATOR*svc,int idx){
    FAVGROUP*grp=(FAVGROUP*)FindFavGroupById(favid);
    if(grp&& (idx<grp->services.size()) && (idx>=0) ){
        *svc=grp->services[idx]; 
        return NGL_OK;
    }
    return NGL_ERROR;
}

int FavAddService(UINT favid,const SERVICELOCATOR*svc){
    FAVGROUP*grp=(FAVGROUP*)FindFavGroupById(favid);
    for(int i=0;i<grp->services.size();i++){
        if(grp->services[i]==*svc)return NGL_ERROR;
    }
    grp->services.push_back(*svc);
    return 0;
}

int FavRemoveService(UINT favid,const SERVICELOCATOR*svc){
    FAVGROUP*grp=(FAVGROUP*)FindFavGroupById(favid);
    for(auto it=grp->services.begin();it!=grp->services.end();it++){
         if((*it)==*svc){
             grp->services.erase(it);
             break;
         }
    }
    return 0;
}

int FavClearService(UINT favid){
    FAVGROUP*grp=(FAVGROUP*)FindFavGroupById(favid);
    grp->services.clear();
    return 0;
}

