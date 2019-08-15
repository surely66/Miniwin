#include <tvdata.h>
#include <si_table.h>
#include <vector>
#include <lrucache.h>
#include <satellite.h>
#include <algorithm>
#include <vector>
#include <lrucache.h>
#include <ngl_log.h>
#include <ngl_dmx.h>
#include <favgroup.h>
#include <map>

NGL_MODULE(TVDATA)

namespace std{
    template<>
    struct hash<SERVICELOCATOR>{
        size_t operator()(SERVICELOCATOR const& s) const{
            return (s.netid<<32)|(s.tsid<<16)|s.sid;
        }
    };
}

static SECTIONLIST epgpf;
static SECTIONLIST bats;
static cache::lru_cache<SERVICELOCATOR ,SECTIONLIST* >epgsCache(64);
static STREAMLIST gStreams;

int AddEITPFSection(const EIT&eit,int *pchanged){
    SECTIONLIST::iterator itr=std::find(epgpf.begin(),epgpf.end(),eit);
    int changed=(itr==epgpf.end());
    if(changed){
        epgpf.push_back(eit);
        std::sort(epgpf.begin(),epgpf.end(),[](PSITable&t1,PSITable&t2){return (EIT&)t1<(EIT&)t2;});
    }
    if(pchanged)*pchanged=changed;
    return epgpf.size();
}

int AddEITSSection(const EIT &eit,int*pchanged){
    const SERVICELOCATOR loc=eit;
    SECTIONLIST* secs=nullptr;
    int changed;
    try{
         secs=epgsCache.get(loc);
         SECTIONLIST::iterator itr=std::find(secs->begin(),secs->end(),eit);
         changed=(itr==secs->end());
         if(changed){
              secs->push_back(eit);
              std::sort(secs->begin(),secs->end(),[](PSITable&t1,PSITable&t2){return (EIT&)t1<(EIT&)t2;});
         }else{
              changed=(itr->crc32()!=eit.crc32());
              *itr=eit;
         }
         if(pchanged)*pchanged=changed;
         return secs->size();
         //NGLOG_VERBOSE_IF(changed,"EITS %d.%d.%d secs=%d",loc.netid,loc.tsid,loc.sid,secs->size());
    }catch(std::range_error e){
         //NGLOG_VERBOSE("create cache for EITS secs=%p %d.%d.%d",secs, loc.netid,loc.tsid,loc.sid);
         secs=new SECTIONLIST();
         epgsCache.put(loc,secs);
         if(pchanged)*pchanged=true;
         return 1;
    }
    return 0;
}

int AddBATSection(const BAT&bat,int*pchanged){
    SECTIONLIST::iterator itr=std::find(bats.begin(),bats.end(),bat);
    bool changed=(itr==bats.end());
    if(changed)
         bats.push_back(bat);
    if(pchanged)*pchanged=changed;
    NGLOG_DEBUG_IF(changed,"rcv BAT %d",bat.extTableId()); 
    return bats.size();
}

STREAMDB*FindStream(USHORT nid,USHORT tsid){
    for(auto ts:gStreams){
        if( (ts.netid==nid) && (ts.tsid==tsid) )
           return &ts;
    }
    return nullptr;
}
int AddStreamDB(const STREAMDB&ts){
    STREAMDB*find=FindStream(ts.netid,ts.tsid);
    if(find){
         NGLOG_ERROR("stream %d.%d exists! add failed",ts.netid,ts.tsid);
         return NGL_ERROR;
    }else{
        if(ts.sdt.size()&&ts.pmt.size()&&ts.pat.size())
            gStreams.push_back(ts);
    }
    return NGL_OK;
}

static size_t SaveSectionList(FILE*f,SECTIONLIST&seclst){
    size_t size=0;
    for(auto s:seclst){
         const BYTE*sec=s;
         NGLOG_DUMP("TABLE",sec,4);
         size_t sz=fwrite(sec,s.sectionLength()+3,1,f);
         size+=sz;
    }
    return size;
}

int DtvLoadProgramsData(const char*fname){
    FILE*f=fopen(fname,"rb");
    char header[8];
    unsigned char sec[4096];
    unsigned short seclen;
    int stream_count;
    if(NULL==f)return 0;
    fread(header,8,1,f);
    if(strncmp(header,"NGL.DATA",8))return 0;
    fread(&stream_count,sizeof(int),1,f);
    NGLOG_DEBUG("streams.size=%d tune.size=%d",stream_count,sizeof(NGLTunerParam));
    for(int i=0;i<stream_count;i++){
        STREAMDB ts;
        fread(&ts.netid,sizeof(USHORT),1,f);
        fread(&ts.tsid,sizeof(USHORT),1,f);
        fread(&ts.tune,sizeof(NGLTunerParam),1,f);
        NGLOG_DEBUG("load ts %d.%d freq:%d",ts.netid,ts.tsid,ts.tune.u.s.frequency);
        do{
            fread(sec,4,1,f);
            NGLOG_DUMP("TABLE",sec,4);
            if(sec[0]==0xFF)break;
            seclen=((sec[1]&0x0F)<<8)|sec[2];
            fread(sec+4,seclen-1,1,f);
            PSITable psi(sec);
            switch(sec[0]){
            case TBID_PAT:ts.pat.push_back(psi);break;
            case TBID_PMT:ts.pmt.push_back(psi);break;
            case TBID_SDT:
            case TBID_SDT_OTHER:ts.sdt.push_back(psi);break;
            default:NGLOG_ERROR("unknown tableid %x",sec[0]);
            }
        }while(!feof(f));
        gStreams.push_back(ts);
    }
    do{
         fread(sec,4,1,f);
         seclen=((sec[1]&0x0F)<<8)|sec[2];
         if(sec[0]!=TBID_BAT)break;
         BAT bat(sec);
         bats.push_back(bat);
    }while(feof(f));
    fclose(f);
    return gStreams.size();
}

int DtvSaveProgramsData(const char*fname){
    FILE*f=fopen(fname,"wb");
    const char *header="NGL.DATA";
    int stream_count=gStreams.size();
    int section_end=0xFFFFFFFF;
    if(NULL==f)return 0;
    fwrite(header,8,1,f);
    fwrite(&stream_count,sizeof(int),1,f);
    for(auto g:gStreams){
        fwrite(&g.netid,sizeof(USHORT),1,f);
        fwrite(&g.tsid,sizeof(USHORT),1,f);
        fwrite(&g.tune,sizeof(NGLTunerParam),1,f);
        SaveSectionList(f,g.pat);
        SaveSectionList(f,g.pmt);
        SaveSectionList(f,g.sdt); 
        fwrite(&section_end,sizeof(int),1,f);
    }
    SaveSectionList(f,bats);
    fclose(f);
    return gStreams.size();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT DtvGetTPByService(const SERVICELOCATOR*loc,TRANSPONDER*tp){
    memset(tp,0,sizeof(NGLTunerParam));
    for(auto t:gStreams){
         if(t.netid==loc->netid&&t.tsid==loc->tsid){
             *tp=t.tune;
             return NGL_OK;
         }
    }
    return NGL_ERROR;
}

INT DtvTuneByService(const SERVICELOCATOR*loc){
    SERVICELOCATOR cur;
    NGLTunerParam tp;
    DtvGetCurrentService(&cur);
    DtvGetTPByService(loc,&tp);
    if(cur.netid!=loc->netid||cur.tsid!=loc->tsid){
         NGLOG_DEBUG("tunning to ts %d.%d %d",loc->netid,loc->tsid,tp.u.s.frequency);
         if(tp.delivery_type==DELIVERY_S||tp.delivery_type==DELIVERY_S2)
            ConfigureTransponder(&tp);
         nglTunerLock(0,&tp);
    }
    return NGL_OK;
}


INT DtvRemoveStreamByTP(int tpid){
    for(auto t=gStreams.begin();t!=gStreams.end();t++){
        if( (t->tune.delivery_type==DELIVERY_S||t->tune.delivery_type==DELIVERY_S2) && t->tune.u.s.tpid==tpid){
            gStreams.erase(t);
            return 0;
        }
    }
    return -1;
}

INT DtvEnumTSService(const STREAMDB&ts,DTV_SERVICE_CBK cbk,void*userdata){
    int rc=0;
    NGLOG_DEBUG("gStreams.size=%d ts.sdt=%d ts.pmt=%d",gStreams.size(),ts.sdt.size(),ts.pmt.size());
    SERVICELOCATOR loc;
    if(ts.sdt.size()){
        char sname[128]={0};
        for(auto itr_sdt:ts.sdt){
            SDT sdt(itr_sdt);
            DVBService svcs[32];
            loc.netid=sdt.getNetId();
            loc.tsid=sdt.getStreamId();
            int cnt=sdt.getServices(svcs,false);
            NGLOG_VERBOSE("service count=%d",cnt);
            for(int i=0;i<cnt;i++){
                svcs[i].getServiceName(sname);
                loc.sid=svcs[i].service_id;
                NGLOG_VERBOSE("%d.%d.%d : %s",loc.netid,loc.tsid,loc.sid,sname);
                rc+=(0!=cbk(&loc,svcs+i,userdata));
            }
        }
    }else if( ts.pat.size() && ts.pmt.size() ){
        PAT pat(ts.pat.front());
        loc.tsid=pat.getStreamId();
        for(auto itr_pmt:ts.pmt){
            PMT pmt(itr_pmt);
            loc.sid=pmt.getProgramNumber();
            rc+=(0!=cbk(&loc,NULL,userdata));
        }
    }
    return rc;
}
INT DtvEnumService(DTV_SERVICE_CBK cbk,void*userdata){
    int rc=0;
     NGLOG_VERBOSE("%d streams",gStreams.size());
    for(auto itr_ts:gStreams ){
        rc+=DtvEnumTSService(itr_ts,cbk,userdata);
    }
    return rc;
}

static void PMT_CBK(DWORD filter,const BYTE *Buffer,UINT BufferLength, void *UserData){
    void**params=(void**)UserData;
    memcpy(params[1],Buffer,BufferLength);
    nglSetEvent((DWORD)params[0]);
   NGLOG_DEBUG("rcv PMT");
}
static int GetPMT(USHORT pid,USHORT sid,BYTE*buffer){
    BYTE mask[8],match[8];
    mask[0]=0xFF;  match[0]=0x02;mask[1]=mask[2]=0;
    mask[3]=0xFF;  match[3]=sid>>8;
    mask[4]=0xFF;  match[4]=sid&0xFF;
    DWORD hevt=nglCreateEvent(0,0);
    void *params[2];
    params[0]=(void*)hevt;
    params[1]=buffer;
    DWORD flt=nglAllocateSectionFilter(0,pid,PMT_CBK,params,NGL_DMX_SECTION);
    nglSetSectionFilterParameters(flt,5,mask,match);
    nglStartSectionFilter(flt);
    int rc=nglWaitEvent(hevt,5000);
    nglFreeSectionFilter(flt);
    nglDestroyEvent(hevt);
    return rc;
}
INT DtvGetServicePmt(const SERVICELOCATOR*sloc,BYTE*pmtbuf){
    int rc=0;
    USHORT pmtpid=0;
    STREAMDB *ts=nullptr;
    NGLOG_DEBUG("streams.size=%d",gStreams.size());
    for(auto itr_ts:gStreams ){
        NGLOG_DEBUG("%d.%d",itr_ts.netid,itr_ts.tsid);
        if(itr_ts.netid==sloc->netid&&itr_ts.tsid==sloc->tsid){
             rc++;   ts=&itr_ts;
             PAT pat(itr_ts.pat.front());
             pmtpid=pat.getPMTPID(sloc->sid);
             for(auto itr_pmt:itr_ts.pmt){
                PMT pmt(itr_pmt);
                if(pmt.getProgramNumber()==sloc->sid){
                    memcpy(pmtbuf,(const BYTE*)pmt,pmt.sectionLength()+3);
                    return pmt.sectionLength()+3;
                }
             }
        }
    }
    if(ts&&pmtpid!=0&&pmtpid!=0x1FFF){
        int rc=GetPMT(pmtpid,sloc->sid,pmtbuf);
        NGLOG_DEBUG("service %d.%d.%d pmt not found ,try getting from live stream=%d",sloc->netid,sloc->tsid,sloc->sid,rc);
        if(NGL_OK==rc){
            PMT psi(pmtbuf,false);
            ts->pmt.push_back(psi);
            return psi.sectionLength()+3;
        }
    }
    NGLOG_DEBUG("%d.%d.%d stream.founded=%d",sloc->netid,sloc->tsid,sloc->sid,rc);
    return 0;
}

INT DtvGetServicePidInfo(const SERVICELOCATOR*sloc,ELEMENTSTREAM*es,USHORT*pcr){
    BYTE pmtbuf[512];
    if(DtvGetServicePmt(sloc,pmtbuf)>0){
        PMT pmt(pmtbuf);
        if(pcr)*pcr=pmt.pcrPid();
        return pmt.getElements(es,true);
    }
    return 0;
}

INT DtvGetPFEvent(const SERVICELOCATOR*sloc,DVBEvent*p,DVBEvent*f){
    int rc=0;
    for(auto itr:epgpf){
        EIT eit(itr);
        if(sloc->netid==eit.getNetId()&&sloc->tsid==eit.getStreamId()&&sloc->sid==eit.getServiceId()){
            if(eit.sectionNo()==0 && eit.getEvents(p,true)==0){
               rc|=1;
            }
            if(eit.sectionNo()==1 && eit.getEvents(f,true)==1){
               rc|=2;
            }
        }
    }
    return rc;
}

INT DtvGetEvents(const SERVICELOCATOR*loc,std::vector<DVBEvent>&evts){
    try{
       int rc=0;
       SECTIONLIST*secs=epgsCache.get(*loc);
       //NGLOG_VERBOSE("EITS %d.%d.%d secs=%d ",loc->netid,loc->tsid,loc->sid,secs->size());
       for(auto t:*secs){
           EIT eit(t);
           SERVICELOCATOR l=eit;
           DVBEvent es[64];
           int cnt=eit.getEvents(es,false);
           for(int i=0;i<cnt;i++){
               evts.push_back(es[i]);
           }
       };
       return evts.size();
    }catch(std::range_error e){
        NGLOG_VERBOSE("no EITS  %d.%d.%d",loc->netid,loc->tsid,loc->sid);
       return 0;
    }
}

INT DtvGetEvents(const SERVICELOCATOR*loc,DVBEvent*evts,int evt_size){
   try{
       int rc=0;
       SECTIONLIST*secs=epgsCache.get(*loc);
       NGLOG_VERBOSE("EITS %d.%d.%d secs=%d ",loc->netid,loc->tsid,loc->sid,secs->size());
       for(auto t:*secs){
           EIT eit(t);
           int cnt=eit.getEvents(evts+rc,true);
           rc+=cnt;
           if(rc>=evt_size)break;
       }
       return rc;
   }catch(std::range_error e){
       NGLOG_VERBOSE("no EITS  %d.%d.%d",loc->netid,loc->tsid,loc->sid);
       return 0;
   }
}


INT DtvCreateGroupByBAT(){
    int count;
    for(auto sec:bats){
        BAT b(sec);
        INT numts;
        char name[64];
        b.getName(name,NULL);
        DVBStream tss[32];
        numts=b.getStreams(tss,false);
        UINT favid=(GROUP_BAT<<16)|b.extTableId();
        if(numts==0)continue;
        FavAddGroupWithID(favid,name);
        count++;
        for(int i=0;i<numts;i++){
            SERVICELOCATOR svcs[32];
            int sc=tss[i].getServices(svcs);
            for(int j=0;j<sc;j++)
                 FavAddService(favid,&svcs[j]);
        }
    }
    return count;
}

class ServiceData:public DVBService{
public:
   BOOL deleted;
   USHORT lcn;
   ServiceData(const BYTE*buf,INT len,bool deep=true):DVBService(buf,len,deep){
       lcn=0xFFFF;
       deleted=FALSE;
   }
   ServiceData(const DVBService&b):DVBService(b){
       lcn=0xFFFF;
       deleted=FALSE;
   }
};

static std::unordered_map<SERVICELOCATOR,ServiceData*>service_lcn;

INT DtvGetServiceItem(const SERVICELOCATOR*svc,SERVICE_KEYITEM item,INT*value){
    std::unordered_map<SERVICELOCATOR,ServiceData*>::const_iterator got=service_lcn.find(*svc);
    if(got==service_lcn.end()||(value==NULL))
        return NGL_ERROR;
    switch(item){
    case SKI_VISIBLE:*value=(got->second->lcn&0x8000)?TRUE:FALSE;break;
    case SKI_DELETED:*value=got->second->deleted;break;
    case SKI_LCN    :*value=(got->second->lcn&0x3FFF);break;
    default:return NGL_ERROR;
    }
    return NGL_OK;
}

const DVBService*DtvGetServiceInfo(const SERVICELOCATOR*svc){
    std::unordered_map<SERVICELOCATOR,ServiceData*>::const_iterator got=service_lcn.find(*svc);
    if(got!=service_lcn.end()){
        return got->second;
    }
    return NULL;
}

class LCNDATA{
public:
   USHORT lcn_start;
   std::unordered_map<SERVICELOCATOR,USHORT>&lcnmap;
   INT getLCN(const SERVICELOCATOR&s,USHORT*lcn){
        std::unordered_map<SERVICELOCATOR,USHORT>::const_iterator got=lcnmap.find(s);
        if(got!=lcnmap.end()){
             *lcn=got->second;
             return NGL_OK;
        }return NGL_ERROR;
   }
};

static INT LCN_CBK(const SERVICELOCATOR*loc,const DVBService*s,void*userdata){
    USHORT lcn;
    LCNDATA *lcndata=(LCNDATA*)userdata;
    if(NGL_OK!=lcndata->getLCN(*loc,&lcn))
       lcn=lcndata->lcn_start++;
    std::unordered_map<SERVICELOCATOR,ServiceData*>::const_iterator got=service_lcn.find(*loc);
    if(got!=service_lcn.end())
        got->second->lcn=lcn;
    return 1;
}

INT DtvInitLCN(USHORT lcn_start){
    int count=0;
    std::vector<SERVICELOCATOR> vsvc;
    std::vector<USHORT> vlcn;
    std::unordered_map<SERVICELOCATOR,USHORT>lcnmap;
    NGLOG_DEBUG("lcn start from %d",lcn_start);
    for(auto sec:bats){
        BAT b(sec);
        SERVICELOCATOR svcs[64];
        USHORT lcns[64];
        DVBStream tss[32];
        int tscount=b.getStreams(tss,false);
        int num=b.getLCN(svcs,lcns);
        if(num>0){//Nordig LCNV1
            count+=num;
            for(int i=0;i<num;i++){vsvc.push_back(svcs[i]);vlcn.push_back(lcns[i]);}
            b.matchServices(vsvc.data(),vsvc.size());
        }else{//Nordig V2
            for(int i=0;i<tscount;i++){
                int sc=tss[i].getLCN(svcs,lcns);
                num+=sc;
                for(int i=0;i<num;i++){vsvc.push_back(svcs[i]);vlcn.push_back(lcns[i]);}
            }
        }
    }
    
    NGLOG_DEBUG("%d service %d lcn",vsvc.size(),vlcn.size());
    for(int i=0;i<vsvc.size();i++){ 
        lcnmap[vsvc[i]]=vlcn[i];
        NGLOG_VERBOSE("\t%d.%d.%d-->%d visible=%d",vsvc[i].netid,vsvc[i].tsid,vsvc[i].sid,(vlcn[i]&0x3FFF),(vlcn[i]&0x8000)!=0);
    }

    LCNDATA lcndata={lcn_start,lcnmap};
    DtvEnumService(LCN_CBK,&lcndata);
    return count;
}

INT DtvGetServiceByLCN(USHORT lcn,SERVICELOCATOR*loc){
    for(auto s:service_lcn){
        if(lcn==s.second->lcn){
            *loc=s.first;
            return NGL_OK;
        }
    }
    return NGL_ERROR;
}

static INT GRP_CBK(const SERVICELOCATOR*loc,const DVBService*s,void*userdata){
    FavAddService(FAV_GROUP_ALL,loc);
    service_lcn[*loc]=new ServiceData(*s);
    switch(s->serviceType){
    case SVC_VIDEO:
         FavAddService(FAV_GROUP_AV,loc);
         FavAddService(FAV_GROUP_VIDEO,loc);
         break;
    case SVC_AUDIO:
         FavAddService(FAV_GROUP_AV,loc);
         FavAddService(FAV_GROUP_AUDIO,loc);
         break; 
    }
    return 1;
}

void DtvCreateSystemGroups(){
    NGLOG_DEBUG("");
    DtvEnumService(GRP_CBK,NULL);
}

