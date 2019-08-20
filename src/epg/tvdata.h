#ifndef __TV_DATA_H__
#define __TV_DATA_H__
#include<ngl_types.h>
#include<si_table.h>

NGL_BEGIN_DECLS

typedef INT (*DTV_SERVICE_CBK)(const SERVICELOCATOR*svcloc,const DVBService*s,void*userdata);
int AddEITPFSection(const EIT&eit,int*changed);
int AddEITSSection(const EIT &eit,int*changed);
int AddBATSection(const BAT&bat,int*changed);
int AddStreamDB(const STREAMDB&ts);
int DtvLoadProgramsData(const char*fname);
int DtvSaveProgramsData(const char*fname);
INT DtvEnumTSService(const STREAMDB&ts,DTV_SERVICE_CBK cbk,void*userdata);

typedef enum{
   SKI_VISIBLE,
   SKI_DELETED,
   SKI_LCN
}SERVICE_KEYITEM;
INT DtvGetServiceItem(const SERVICELOCATOR*svc,SERVICE_KEYITEM item,INT*value);
const DVBService*DtvGetServiceInfo(const SERVICELOCATOR*svc);
INT DtvGetServiceByLCN(USHORT lcn,SERVICELOCATOR*loc);

typedef enum{
  LCN_FROM_NIT=1,
  LCN_FROM_BAT=2,
  LCN_FROM_USER=4
}LCNMODE;

INT DtvInitLCN(LCNMODE mode,USHORT lcnstart);

INT DtvCreateGroupByBAT();
void DtvCreateSystemGroups();

NGL_END_DECLS

#endif
