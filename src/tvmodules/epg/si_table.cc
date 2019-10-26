#include<si_table.h>
#include<ngl_video.h>
#include<ngl_os.h>
#include<stdarg.h>
#include<stdio.h>
#include<ngl_log.h>
#include<utils.h>

NGL_MODULE(SITABLE)


PSITable::PSITable(const BYTE*buf,bool deepcopy){
    int slen=(buf[1]&0x1F)<<8|buf[1];
    data=(unsigned char*)buf;
    owndata=deepcopy;
    if(deepcopy){
        data=(BYTE*)nglMalloc(slen+3);
        memcpy(data,buf,slen+3);
    }
}

PSITable::PSITable(const PSITable& b,bool deepcopy){
    const BYTE*buf=b.data;
    int slen=b.sectionLength(); 
    owndata=deepcopy;
    data=b.data;
    if(deepcopy){
        data=(BYTE*)nglMalloc(b.sectionLength()+3);
        memcpy(data,b.data,b.sectionLength()+3);
    }
}

PSITable::~PSITable(){
   NGLOG_DEBUG_IF(data==nullptr,"data is null");
   if(data&&owndata)nglFree(data);
   data=nullptr;
}

PSITable& PSITable::operator=(const PSITable &b){
    size_t blen=b.sectionLength()+3;
    data=(BYTE*)nglRealloc(data,blen);
    memcpy(data,b.data,blen);
    return *this;
}

bool PSITable::operator!=(PSITable & b)const {
    return extTableId()!=b.extTableId()||tableId()!=b.tableId()||sectionNo()!=b.sectionNo();
}

bool PSITable::operator>=(PSITable & b)const {
    return extTableId()>=b.extTableId()||tableId()>=b.tableId()||sectionNo()>=b.sectionNo();
}

bool PSITable::operator<=(PSITable & b)const {
    return extTableId()<=b.extTableId()||tableId()<=b.tableId()||sectionNo()<=b.sectionNo();
}

bool PSITable::operator>(PSITable & b)const {
    return extTableId()>b.extTableId()||tableId()>b.tableId()||sectionNo()>b.sectionNo();
}

bool PSITable::operator<(PSITable & b)const {
    long l1=(extTableId()<<16)|(tableId()<<8)|sectionNo();
    long l2=(b.extTableId()<<16)|(b.tableId()<<8)|b.sectionNo();
    return l1<l2;//extTableId()<b.extTableId()||sectionNo()<b.sectionNo()||(tableId()&0xC0)<(b.tableId()&0xC0);
}

bool PSITable::operator==(const PSITable & b)const {
    switch(tableId()){
    case 0x4E:
    case 0x4F:
         return ((tableId()&0x4E)==(b.tableId()&0x4E))&&extTableId()==b.extTableId()&&sectionNo()==b.sectionNo();
    case 0x50 ... 0x6F:
       return ((tableId()&0xCF)==(b.tableId()&0xCF))&&extTableId()==b.extTableId()&&sectionNo()==b.sectionNo();
    default:return tableId()==b.tableId()&&extTableId()==b.extTableId()&&sectionNo()==b.sectionNo();
    }
}

int PAT::getPrograms(PROGRAMMAP*maps,bool nit){
    BYTE*p=data+8;
    PROGRAMMAP*m=maps;
    for(;p<data+sectionLength()-1;p+=4){
        m->program_number=p[0]<<8|p[1];
        m->pmt_pid=(p[2]&0x1F)<<8|p[3];
        if(0!=m->program_number||nit){m++;}
    }
    return m-maps;
}

int PAT::getPMTPID(USHORT program_number){
    PROGRAMMAP prgs[32];
    int cc=getPrograms(prgs,false);
    for(int i=0;i<cc;i++){
       if(prgs[i].program_number==program_number)
          return prgs[i].pmt_pid;
    }
    return 0x1FFF;
}
INT MpegElement::getType(){
    switch(stream_type){
    case 1:return DECV_MPEG;
    case 2:return DECV_MPEG;
    case 3:return DECA_MPEG1;
    case 4:return DECA_MPEG2;
    default:NGLOG_INFO("===TODO pid=%d stream_type=%d",pid,stream_type);
    }
    if(findDescriptors(TAG_AC3,TAG_ENHANCED_AC3))
        return DECA_AC3;
    if(findDescriptor(TAG_DTS))
        return DECA_DTS;
    if(findDescriptor(TAG_AAC))
        return DECA_AAC_LATM;//??
    return 0;
}

INT MpegElement::getCategory(){
    switch(stream_type){
    case 1:
    case 2:return ST_VIDEO;
    case 3:
    case 4:return ST_AUDIO;
    case 5:
    case 6:
         if(findDescriptors(TAG_AC3,TAG_ENHANCED_AC3,TAG_DTS,TAG_AAC))
            return ST_AUDIO;
    case 7:return ST_DATA;//used for MHEG
    case 8:return ST_DATA;
    case 0x0A ... 0x0D:
           return ST_DATA;
    }
    return -1;
}

INT MpegElement::getCAECM(USHORT*caid,USHORT*ecmpid){
    INT len,cnt=0;
    BYTE*p=descriptors;
    for(;p-descriptors<length;){
        if(p[0]==TAG_CA){
           if(caid)*caid ++=(p[2]<<8)|p[3];
           if(ecmpid)*ecmpid++=(p[4]&0x1F)<<8|p[5];
           cnt++;
        }p+=(2+p[1]);
    }
    return cnt;

}

INT PMT::getCAECM(USHORT*caid,USHORT*ecmpid){
    INT len,cnt=0;
    BYTE*p,*pinfo=getProgramDescriptors(len);
    for(p=pinfo;p-pinfo<len;){
        if(p[0]==TAG_CA){
           if(caid)  *caid++=(p[2]<<8)|p[3];
           if(ecmpid)*ecmpid++=(p[4]&0x1F)<<8|p[5];
           cnt++;
        }p+=(2+p[1]); 
    }
    return cnt;
}

BYTE*CAT::getDescriptors(int&len){
    len=sectionLength()-9;
    return data+8;
}
USHORT CAT::getEMMPID(){
    USHORT pid=0x1FFF;
    Descriptors des(data+8,sectionLength()-9);
    BYTE*p=des.findDescriptor(TAG_CA);
    if(p)pid=(p[4]&0x1F)<<8|p[5];
    return pid;
}

static void GetExtESInfo(ELEMENTSTREAM*es){
    BYTE*pd=es->findDescriptor(TAG_ISO639_LANGUAGE);
    memset(es->iso639lan,0,sizeof(es->iso639lan));
    if(pd)
        memcpy(es->iso639lan,pd+2,3);
}
int PMT::getElements(ELEMENTSTREAM*es,bool own){
    INT i,len1,len2;
    BYTE*des=getProgramDescriptors(len1);
    des+=len1;
    for(i=0;des<data+sectionLength()-1;i++){
        char*l=es->iso639lan;
        es->stream_type=des[0];
        es->pid=(des[1]&0x1F)<<8|des[2];
        es->setDescriptor(des+5,(des[3]&0x0F)<<8|des[4],own);//es->length=(des[3]&0x0F)<<8|des[4];
        des+=es->getLength()+5;
        GetExtESInfo(es);
        es++;
    }
    return i;
}

int SDT::getServices(DVBService*services,bool own){
    DVBService*s=services;
    for(BYTE*pd=data+11;pd<data+sectionLength()-1;s++){
        s->netid=getNetId();
        s->tsid=getStreamId();
        s->service_id=pd[0]<<8|pd[1];
        s->eitS=(pd[2]>>1)&0x01;
        s->eitPF=pd[2]&0x01;
        s->runStatus=pd[3]>>5;
        s->freeCAMode=(pd[3]>>4)&1;
        s->setDescriptor(pd+5,(pd[3]&0x0F)<<8|pd[4],own);//length=(pd[3]&0x0F)<<8|pd[4];
        BYTE*p1=s->findDescriptor(TAG_SERVICE);
        if(p1)s->serviceType=p1[2];
        pd+=s->getLength()+5;//
    }
    return (s-services);
}

DVBService::DVBService():Descriptors(){
}

DVBService::DVBService(const DVBService&b):Descriptors(b){
    netid=b.netid;
    tsid=b.tsid;
    service_id=b.service_id;
    eitS=b.eitS;
    eitPF=b.eitPF;
    runStatus=b.runStatus;
    freeCAMode=b.freeCAMode;
    BYTE*p1=findDescriptor(TAG_SERVICE);
    if(p1)serviceType=p1[2];
}

DVBService::DVBService(const BYTE*buf,INT len,bool deep)
   :Descriptors(buf,len,deep){
}

int DVBService::getServiceName(char*name,char*provider)const{
     BYTE*pd=findDescriptor(TAG_SERVICE);
     if(pd==NULL)return 0;
     ServiceDescriptor sd(pd,pd[1],false);
     return sd.getName(name,provider);
}

DVBEvent::DVBEvent():Descriptors(){
    event_id=0;
    start_time=0;
    duration=0;
}

DVBEvent::DVBEvent(const DVBEvent&o):Descriptors(o){
    event_id=o.event_id;
    start_time=o.start_time;
    duration=o.duration; 
}

int DVBEvent::getShortName(char*name,char*des){
    BYTE*p=findDescriptor(TAG_SHORT_EVENT);
    if(p){
         ShortEventNameDescriptor sd(p,p[1],false);
         return sd.getName(name,des);
    }
    return 0;
}

int EIT::getEvents(EVENT*events,bool own){
     EVENT*e=events;
     for(BYTE*pd=data+14;pd<data+sectionLength()-1;e++){
         int mjd,utc;
         e->event_id=pd[0]<<8|pd[1];
         mjd=pd[2]<<8|pd[3];
         utc=pd[4]<<16|pd[5]<<8|pd[6];
         UTC2Time(mjd,utc,&e->start_time);
         e->duration=Hex2BCD(pd[7]<<16|pd[8]<<8|pd[9]);
         e->setDescriptor(pd+12,((pd[10]&0x0f)<<8)|pd[11],own);//length=((pd[10]&0x0f)<<8)|pd[11];
         pd+=12+ e->getLength();
     }
     return e-events;
}

EIT::operator const SERVICELOCATOR()const{
    SERVICELOCATOR l;
    l.netid=getNetId();
    l.tsid=getStreamId();
    l.sid=getServiceId();
    l.tpid=0xFFFF;
    return l;
}

bool DVBStream::getDelivery(NGLTunerParam*tp){
  /*TAG_SATELLITE_DELIVERY 0x43
  TAG_CABLE_DELIVERYY    0x44
  TAG_TERRSTRIAL_DELIVERY 0x5A
  TAG_SATELLITE2_DELIVERY 0x79*/
    BYTE*p=findDescriptor(TAG_CABLE_DELIVERYY);
    if(p){//frequency in DVBC is Mhz
        tp->u.c.frequency=Hex2BCD((p[2]<<24)|(p[3]<<16)|(p[4]<<8)|p[5]);
        tp->u.c.modulation=(NGLQamMode)p[8];
        tp->u.c.symbol_rate=Hex2BCD((p[9]<<20)|(p[10]<<12)|(p[11]<<4)|(p[12]>>4));
        //fecinner=p[12]&0x0F;
        return 1;
    }else if(p=findDescriptor(TAG_SATELLITE_DELIVERY)){//frequency for DVBS Ghz
        tp->u.s.frequency=Hex2BCD((p[2]<<24)|(p[3]<<16)|(p[4]<<8)|p[5]);
        tp->u.s.position=Hex2BCD(p[6]<<8|p[7]);
        tp->u.s.direction=p[8]>>7;
        tp->u.s.polar=(NGLNimPolar)( (p[8]>>5)&0x03);
        tp->u.s.symbol_rate=Hex2BCD((p[9]<<20)|(p[10]<<12)|(p[11]<<4)|(p[12]>>4));
        tp->u.s.modulation=(NGLModulation)(p[8]&0x03);
        tp->u.s.fec=(NGLNimFEC)(p[12]&0x0F);
        return 1;
    }else if(p=findDescriptor(TAG_TERRSTRIAL_DELIVERY)){//frequency for DVBT is 10Hz
        tp->u.t.frequency=Hex2BCD((p[2]<<24)|(p[3]<<16)|(p[4]<<8)|p[5]);
        tp->u.t.bandwidth=(NGLBandWidth)(p[6]>>5);
        tp->u.t.guard_interval=(NGLGuardInter)((p[8]>>3)&3);
        //tp->u.t.modulation
        tp->u.t.guard_interval=(NGLGuardInter)((p[8]>>3)&0x03);//tobe convert
        return 1;
    }else if(p=findDescriptor(TAG_SATELLITE2_DELIVERY)){
        //tp->frequency=(p[2]<<24)|(p[3]<<16)|(p[4]<<8)|p[5];
    }
    return 0;
}

int NIT::getStreams(STREAM*ts,bool own ){
    int len;
    BYTE*pd=getNetworkDescriptors(len);
    STREAM*ts0=ts;
    int tslen=((pd[len]&0x0F)<<8)|pd[len+1];
    for(BYTE*pd=data+12+len;tslen>0;ts++){
        ts->tsid=pd[0]<<8|pd[1];
        ts->netid=pd[2]<<8|pd[3];
        int slen=((pd[4]&0x0F)<<8)|pd[5];
        ts->setDescriptor(pd+6,slen,own);//length=((pd[4]&0x0F)<<8)|pd[5];
        pd+=6+ts->getLength();
        tslen-=(6+ts->getLength());
    }
    return ts-ts0;
}

static int SetNIDTSBySID(SERVICELOCATOR*svcs,int size,USHORT sid,USHORT nid,USHORT tid){
    for(int i=0;i<size;i++){
       if(svcs[i].sid==sid){
           svcs[i].netid=nid;
           svcs[i].tsid=tid;
           return 1;
       }
    }return 0;
}

int NIT::matchServices(SERVICELOCATOR*svc,int size){
    STREAM tss[32];
    int ns,rc=0;
    int nts=getStreams(tss,false);
    for(int i=0;i<nts;i++){
         SERVICELOCATOR sss[32];
         ns=tss[i].getServices(sss);
         for(int j=0;j<ns;j++){
             rc+=SetNIDTSBySID(svc,size,sss[j].sid, tss[i].netid,tss[i].tsid);
         }
    }
    return rc;
}

int NIT::getLCN(SERVICELOCATOR*svc,USHORT*lcn){
    int len;
    BYTE*pd=getNetworkDescriptors(len);
    Descriptors dess(pd,len);
    if(pd=dess.findDescriptor(TAG_NORDIG_LCN)){
        NordigLCNDescriptor nd(0,0,pd,pd[1],false);
        return nd.getLCN(svc,lcn);
    }
    if(pd=dess.findDescriptor(TAG_NORDIG_LCN2)){
        NordigLCNDescriptorV2 nd(0,0,pd,pd[1],false);
        return nd.getLCN(svc,lcn);
    }
    return 0;
}

int DVBStream::getServices(SERVICELOCATOR*svc){
    BYTE*pd=findDescriptor(TAG_SERVICE_LIST);
    if(nullptr!=pd){
        ServiceListDescriptor sd(netid,tsid,pd,pd[1],false);
        return sd.getService(svc,NULL);
    }
    return 0;
}

int DVBStream::getLCN(SERVICELOCATOR*svc,USHORT*lcn){
    BYTE*pd=findDescriptor(TAG_NORDIG_LCN);
    if(nullptr!=pd){
        NordigLCNDescriptor nd(netid,tsid,pd,pd[1],false);
        return nd.getLCN(svc,lcn);
    }
    if(pd=findDescriptor(TAG_NORDIG_LCN2)){
        NordigLCNDescriptorV2 nd(netid,tsid,pd,pd[1],false);
        return nd.getLCN(svc,lcn);
    }
    return 0;
}

int NIT::getName(char*name,const char*lan){
    int len,rc=0;
    BYTE*pd=getNetworkDescriptors(len);
    Descriptors des(pd,len,false);
    if(pd=des.findDescriptor(TAG_MULTI_NETWORK_NAME)){
        MultiBouquetNameDescriptor md(pd,pd[1],false);
        return md.getName(name,lan);
    }
    if(pd=des.findDescriptor(TAG_NETWORK_NAME)){
        NameDescriptor sd(pd,pd[1],false);
        return sd.getName(name);
    }
    return 0;
}

int BAT::getName(char*name,const char*lan){
    int len;
    BYTE*pd=getBouquetDescriptors(len);
    Descriptors des(pd,len,false);
    if(pd=des.findDescriptor(TAG_MULTI_BOUQUET_NAME)){
        MultiBouquetNameDescriptor md(pd,pd[1],false);
        return md.getName(name,lan);
    }
    if( pd=des.findDescriptor(TAG_BOUQUET_NAME) ){
        NameDescriptor sd(pd,pd[1],false);
        return sd.getName(name);
    }
    return 0;
}

BYTE*TOT::getDescriptors(int&len){
     len=(data[8]&0x0F)<<8|data[9];
     return data+10;
}

int TOT::getTimeOffset(TimeOffset*tms,int size){
    int len;
    BYTE*p=getDescriptors(len);
    for(;len>0;len-=13,p+=13){
         UINT mjd,utc;
         memcpy(tms->country_code,p,3);
         tms->country_code[3]=0;
         tms->region_id=p[3]>>2;
         tms->polarity=p[3]&1;
         tms->local_time_offset=p[4]<<8|p[5];
         mjd=p[6]<<8|p[7];
         utc=Hex2BCD(p[8]<<16|p[9]<<8|p[10]);
         UTC2Time(mjd,utc,&tms->time_of_change);
         tms->next_time_offset=p[11]<<8|p[12];
         tms++;
    }
    return len/13;
}
