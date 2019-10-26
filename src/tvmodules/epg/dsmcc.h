#ifndef __DSMCC_H__
#define __DSMCC_H__
#include <si_table.h>
class DSMCC:public PSITable{
public:
    DSMCC(const BYTE*buf,bool deepcopy=true):PSITable(buf,deepcopy){}
    DSMCC(const PSITable&b,bool deepcopy=true):PSITable(b,deepcopy){}
    USHORT transactionId(){return extTableId();}
};

class DSI:public DSMCC{
public:
  DSI(const BYTE*buf,bool deepcopy=true):DSMCC(b,deepcopy){}
  DSI(const PSITable&b,bool deepcopy=true):DSMCC(b,deepcopy){}
};

class DII:public DSMCC{
public:
  DII(const BYTE*buf,bool deepcopy=true):DSMCC(b,deepcopy){}
  DII(const PSITable&b,bool deepcopy=true):DSMCC(b,deepcopy){}
};

class DDB:public DSMCC{
public:
  DDB(const BYTE*buf,bool deepcopy=true):DSMCC(b,deepcopy){}
  DDB(const PSITable&b,bool deepcopy=true):DSMCC(b,deepcopy){}
};

#endif
