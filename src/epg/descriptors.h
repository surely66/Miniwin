#ifndef __DVBSI_DESCRIPTOR_H__
#define __DVBSI_DESCRIPTOR_H__
#include<destags.h>
#include<ngl_types.h>
///////////////////////////////////////////////////////////////////////////////////////
typedef struct ServiceLocator{
   USHORT netid;//original network id or tp(for DVBS)
   USHORT tsid; //transport stream id
   USHORT sid;  //service id
   USHORT tpid; //for satellite tp
   bool operator==(const ServiceLocator&b)const{return netid==b.netid && tsid==b.tsid && sid==b.sid;}
   ServiceLocator&operator=(const ServiceLocator &b){
      netid=b.netid;tsid=b.tsid;sid=b.sid;tpid=b.tpid;
   }
}SERVICELOCATOR;

class Descriptors{
private:
   int ownedbuff;
protected:
   INT length;
   BYTE*descriptors;
public:
   Descriptors();
   Descriptors(const Descriptors&o);
   Descriptors(const BYTE*des,INT len,bool deepcopy=false);
   ~Descriptors();
   void setDescriptor(const BYTE*des,INT len,bool deepcopy=false);
   void cloneData();
   BYTE*findDescriptor(INT tag)const;
   INT findDescriptors(INT,...)const;
   int getLength()const{return length;}
   operator const BYTE*(){return descriptors;}
};

class CADescriptor:Descriptors{
public:
    CADescriptor(const BYTE*pd,int len,bool deep=false):Descriptors(pd,len,deep){}
    USHORT getCAID(){return descriptors[2]<<8|descriptors[3];}
    USHORT getEcmPID(){return ((descriptors[4]&0x1F)<<8)|descriptors[5];}
};
class NameDescriptor:public Descriptors{
public:
    NameDescriptor(const BYTE*pd,int len,bool deep=false):Descriptors(pd,len,deep){}
    int getName(char*name);
};
class MultiNameDescriptor:public Descriptors{
public:
    MultiNameDescriptor(const BYTE*pd,int len,bool deep=false):Descriptors(pd,len,deep){}
    int getName(char*name,const char*lan);
};
typedef NameDescriptor BouquetNameDescriptor;
typedef NameDescriptor NetworkNameDescriptor;
typedef MultiNameDescriptor MultiNetworkNameDescriptor;
typedef MultiNameDescriptor MultiBouquetNameDescriptor;

class ServiceDescriptor:Descriptors{
public:
    ServiceDescriptor(const BYTE*pd,int len,bool deep=false):Descriptors(pd,len,deep){}
    BYTE getServiceType(){return descriptors[2];}
    int getName(char*name,char*provider);
};

class MultiServiceNameDescriptor:public Descriptors{
public:
    MultiServiceNameDescriptor(const BYTE*pd,int len,bool deep=false):Descriptors(pd,len,deep){}
    int getName(char*name,char*provider,const char*lan);
};

class ShortEventNameDescriptor:Descriptors{
public:
    ShortEventNameDescriptor(const BYTE*pd,int len,bool deep=false):Descriptors(pd,len,deep){}
    void getLanguage(char*lan);
    int getName(char*name,char*desc);
};


class ExtendEventDescriptor:public Descriptors{
public:

    ExtendEventDescriptor(const BYTE*pd,int len,bool deep=false):Descriptors(pd,len,deep){}
    int getDescriptorNumber(){return descriptors[2]>>4;}
    int getLastDescriptorNumber(){return descriptors[2]&0x0F;}
    int getItemLength(){return descriptors[6];}
    void getLanguage(char*lan);
    int getText(char*text);
};

class ParentRatingDescriptor:public Descriptors{
public:
    ParentRatingDescriptor(const BYTE*pd,int len,bool deep=false):Descriptors(pd,len,deep){}
    int getRatings(BYTE*clan);
    int getRating(const char*lan);
};

class ServiceListDescriptor:public Descriptors{
protected:
    USHORT netid,tsid;
public:
    ServiceListDescriptor(USHORT netid,USHORT tsid,const BYTE*pd,int len,bool deep=false);
    int getService(SERVICELOCATOR*svc,BYTE*service_type=nullptr);
};

class NordigLCNDescriptor:public Descriptors{//0x83 NordigLCN V1
protected:
    USHORT netid,tsid;
public:
    NordigLCNDescriptor(USHORT netid,USHORT tsid,const BYTE*pd,int len,bool deep=false);
    //lcn only used 14bit :1:visibleflag 1:reserved 14:lcn
    virtual int getLCN(SERVICELOCATOR*svc,USHORT*lcn);
};

class NordigLCNDescriptorV2:NordigLCNDescriptor{
public://used int BAT NIT in second descriptors' loop
    NordigLCNDescriptorV2(USHORT netid,USHORT tsid,const BYTE*pd,int len,bool deep=false);
    int getLCN(SERVICELOCATOR*svc,USHORT*lcn);
};

#endif //enf of _DVBSI_DESCRIPTORS
