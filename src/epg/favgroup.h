#ifndef __FAV_GROUP_H__
#define __FAV_GROUP_H__
#include <ngl_types.h>
#include <ngl_log.h>
#include <descriptors.h>
NGL_BEGIN_DECLS

//system default favorite group id
/*favid=group_type<<16|id ,it is unique in system
 for bat id =bouquet_id grouptype=GROUP_BAT
 for favorite group grouptype=GROUP_FAV 
 for system grouptype=0
*/
#define GROUP_SYSTEM 0x8000
#define GROUP_CUSTOM 0x4000
#define GROUP_BAT    0x2000
#define GROUP_FAV    0x0000

#define FAV_GROUP_ALL   ((GROUP_SYSTEM<<16)|0)
#define FAV_GROUP_AV    ((GROUP_SYSTEM<<16)|1)
#define FAV_GROUP_VIDEO ((GROUP_SYSTEM<<16)|2)
#define FAV_GROUP_AUDIO ((GROUP_SYSTEM<<16)|3)

int FavInit(const char*favpath);
int FavLoadData(const char*fname);
int FavSaveData(const char*fname);
int FavGetGroupCount();
int FavGetGroupInfo(int idx,UINT*id,char*grpname);
int FavGetGroupName(UINT favid,char*name);
//only for custom favorite return favid
int FavAddGroup(const char*name);
int FavAddGroupWithID(UINT favid,const char*name);
int FavRemoveGroup(int favid);

UINT FavGetServiceCount(UINT favid);
int FavGetServices(UINT favid,SERVICELOCATOR*svcs,int count);
int FavGetService(UINT favid,SERVICELOCATOR*svcs,int idx);
int FavAddService(UINT favid,const SERVICELOCATOR*svc);
int FavRemoveService(UINT favid,const SERVICELOCATOR*svc);
int FavClearService(UINT favidgrp);

NGL_END_DECLS
#endif
