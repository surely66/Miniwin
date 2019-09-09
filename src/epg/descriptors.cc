#include<stdarg.h>
#include<stdio.h>
#include<string.h>
#include<descriptors.h>
#include<ngl_os.h>
#include<utils.h>

Descriptors::Descriptors(){
    ownedbuff=0;
    descriptors=NULL;
    des_length=0;
}

Descriptors::Descriptors(const Descriptors&o)
  :Descriptors(){
    setDescriptor(o.descriptors,o.des_length,true);
}

Descriptors::Descriptors(const BYTE*des,INT len,bool deepcopy)
  :Descriptors(){
     setDescriptor(des,len,deepcopy);
}

Descriptors::~Descriptors(){
    if(ownedbuff && descriptors)
        nglFree(descriptors);
    descriptors=NULL;
}

void Descriptors::cloneData(){
    if(des_length && 0==ownedbuff){
       BYTE*p=(BYTE*)nglMalloc(des_length);
       memcpy(p,descriptors,des_length);
       descriptors=p;
       ownedbuff=1;
    }
}

void Descriptors::setDescriptor(const BYTE*des,INT len,bool deepcopy){
    if(ownedbuff && descriptors)
         nglFree(descriptors);
    if(deepcopy){
       descriptors=(BYTE*)nglMalloc(len);
       memcpy(descriptors,des,len);
    }else{
       descriptors=(BYTE*)des;
    }
    des_length=len;
    ownedbuff=deepcopy;
}

BYTE*Descriptors::findDescriptor(INT tag)const{
    for(int pos=0;pos<des_length;){
        if(descriptors[pos]==tag)return descriptors+pos;
        pos+=2+descriptors[pos+1];
    }
    return NULL;
}

INT Descriptors::findDescriptors(INT tag,...)const
{
    int found=0;
    va_list ap;
    va_start(ap,tag);
    do{
       tag=va_arg(ap,int);
       found+=(NULL!=findDescriptor(tag));
    }while(tag);
    va_end(ap);
    return found;
}

int NameDescriptor::getName(char*name){
    ToUtf8((const char*)(descriptors+2),des_length,name);
    return descriptors[1];
}

int ServiceDescriptor::getName(char*name,char*provider){
     BYTE*pd=descriptors;
     int plen=pd[3];
     int slen=pd[4+plen];
     if(provider)ToUtf8((const char*)(pd+4),plen,provider);
     if(name)    ToUtf8((const char*)(pd+5+plen),slen,name);
     return (slen<<16)|plen;
}

int MultiServiceNameDescriptor::getName(char*name,char*provider,const char*lan){
    BYTE*p=descriptors+2;
    for(int i=0;i<des_length;i++){
        char nlan[3];
        memcpy(nlan,p,3);
        int plen=p[3];
        int slen=p[4+plen];
        if((lan==nullptr)||(memcmp(p,lan,3)==0)){
            if(provider)ToUtf8((const char*)(p+4),plen,provider);
            if(name)ToUtf8((const char*)(p+5+plen),slen,name);
            return (slen<<16)|plen;
        }
        p+=(5+plen+slen);
    }   
    return 0;
}

int ParentRatingDescriptor::getRatings(BYTE*clan){
    memcpy(clan,descriptors,des_length);
    return des_length/4;
}

int ParentRatingDescriptor::getRating(const char*lan){
    for(int i=0;i<des_length;i+=4){
        if(memcmp(descriptors+i+2,lan,3)==0)
           return descriptors[+i+5];
    }
    return 0;
}

void ShortEventNameDescriptor::getLanguage(char*lan){
    memcpy(lan,descriptors+2,3);
}

int ShortEventNameDescriptor::getName(char*name,char*desc){
    BYTE*p=descriptors;
    if(name)name[0]=0;
    if(desc)desc[0]=0;
    if(p){
        int nmlen=p[5];
        ToUtf8((const char*)(p+6),nmlen,name); 
        int dlen=p[6+nmlen];
        ToUtf8((const char*)(p+7+nmlen),dlen,desc);
        return nmlen<<16|dlen;
    }
    return 0;
}

void ExtendEventDescriptor::getLanguage(char*lan){
    memcpy(lan,descriptors+3,3);
}

int  ExtendEventDescriptor::getText(char*text){
    BYTE*p=descriptors+(getItemLength()+7);
    return ToUtf8((const char*)(p+1),p[0],text);
}

int MultiNameDescriptor::getName(char*name,const char*lan){
    BYTE*p=descriptors+2;
    for(int i=0;i<des_length;){
        char nlan[4];
        memcpy(nlan,p,3);
        int nlen=p[3];
        if(lan==NULL||memcmp(nlan,lan,3)==0){
            ToUtf8((const char*)(p+4),nlen,name);
            return nlen;
        }
        p+=(3+nlen);
    }
    return 0;
}

ServiceListDescriptor::ServiceListDescriptor(USHORT nid,USHORT tid,const BYTE*pd,int len,bool deep)
   :Descriptors(pd,len,deep){
    netid=nid;
    tsid=tid;
}

int ServiceListDescriptor::getService(SERVICELOCATOR*svc,BYTE*types){
    BYTE*pd=descriptors+2;
    int count=des_length/3;
    for(int i=0;i<count;i++){
        svc[i].netid=netid;
        svc[i].tsid=tsid;
        svc[i].sid=(pd[0]<<8)|pd[1];
        if(types)types[i]=pd[2];
        pd+=3;
    }
    return count;
}

NordigLCNDescriptor::NordigLCNDescriptor(USHORT nid,USHORT tid,const BYTE*pd,int len,bool deep)
   :Descriptors(pd,len,deep){
    netid=nid;
    tsid=tid;
}

int NordigLCNDescriptor::getLCN(SERVICELOCATOR*svc,USHORT*lcn){
    BYTE*pd=descriptors+2;
    int count=des_length/4;
    for(int i=0;i<count;i++){
        svc[i].netid=netid;
        svc[i].tsid=tsid;
        svc[i].sid=(pd[0]<<8)|pd[1];
        if(lcn)lcn[i]=(pd[2]<<8)|pd[3];
        pd+=4;
    }
    return count;
}

NordigLCNDescriptorV2::NordigLCNDescriptorV2(USHORT nid,USHORT tid,const BYTE*pd,int len,bool deep)
   :NordigLCNDescriptor(nid,tid,pd,len,deep){
}

int NordigLCNDescriptorV2::getLCN(SERVICELOCATOR*svc,USHORT*lcn){
    BYTE*pd=descriptors+2;
    int idx=0;
    for(;pd<descriptors+getLength();){
        BYTE chid=pd[0];
        BYTE chnamelen=pd[1];
        BYTE country[3];
        memcpy(country,pd+2,3);
        int len=pd[5+chnamelen];
        pd+=6+chnamelen;
        for(int i=0;i<len;i+=4,pd+=4,idx++){
            svc[idx].netid=netid;
            svc[idx].tsid=tsid;
            svc[idx].sid=(pd[0]<<8)|pd[1]; 
            lcn[idx]=(pd[2]<<8)|pd[3];
        }
    }
    return idx;
}

