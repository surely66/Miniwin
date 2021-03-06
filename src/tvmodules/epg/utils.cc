#include <iconv.h>//libc hass iconv.h
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <ngl_log.h>
#include <ngl_os.h>
#include <ctype.h>
#include <errno.h>
#include <dlfcn.h>
NGL_MODULE(UTILS)

#ifdef __GLIBC__
/**__GLIBC__has include iconv but has some problem, so we load func from so*/
typedef iconv_t (*_iconv_open)(const char* tocode, const char* fromcode);
typedef size_t (*_iconv) (iconv_t cd,  char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
typedef int (*_iconv_close) (iconv_t cd);
#endif


class CodeConverter {
private:
    iconv_t cd;
#ifdef __GLIBC__
static void*sohandle;
static _iconv_open iconvopen;
static _iconv iconv_conv;
static _iconv_close iconvclose;
#endif
public:
    CodeConverter(const char *from_charset,const char *to_charset) {
#ifdef __GLIBC__
        if(nullptr==sohandle){
            sohandle=dlopen("libiconv.so",RTLD_LAZY);
            iconvopen=(_iconv_open)dlsym(sohandle,"libiconv_open");
            iconv_conv=(_iconv)dlsym(sohandle,"libiconv");
            iconvclose=(_iconv_close)dlsym(sohandle,"libiconv_close");
            NGLOG_DEBUG_IF((sohandle==NULL)||(iconvopen==NULL)||(iconv_conv==NULL)||(iconvclose==NULL),
                  "iconv_open=%d errno=%d sohdl=%p funcs=%p/%p/%p",cd,errno,sohandle,iconvopen,iconv_conv,iconvclose);
        }
        cd =iconvopen(to_charset,from_charset);
#else
        cd =iconv_open(to_charset,from_charset);
#endif
        NGLOG_DEBUG_IF(-1L==(long)cd,"iconv_open=%d errno=%d sohdl=%p",cd,errno);
    }
    ~CodeConverter() {
#ifdef __GLIBC__
        iconvclose(cd);
#else
        iconv_close(cd);
#endif 
    }
    int convert(char *inbuf,int len,char *outbuf,int outlen) {
        char**pin=&inbuf,**pout=&outbuf,*ins=inbuf;
        int lenin=outlen,inlen=len;
#ifdef __GLIBC__
        int rc=iconv_conv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
#else
        int rc=iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
#endif
        //CheckUTF8((BYTE*)ins,(BYTE*)outbuf,lenin-outlen);
        return lenin-outlen;
    }
   int CheckUTF8(BYTE*inbuf,BYTE*str,int len){
/*1字节 0xxxxxxx
  2字节 110xxxxx 10xxxxxx
  3字节 1110xxxx 10xxxxxx 10xxxxxx
  4字节 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  5字节 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
  6字节 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
       unsigned char*u8=str;
       int u8err=0;
       for(;u8-str<len;){
           int bytes=0;
           if (*u8>>7==0x0){//1 BYTE
               bytes=1;
           }else if(*u8>>5==0x6){//2 BYTE
               bytes=2;
           }else if(*u8>>4==0x0E){//3 BYTE 
               bytes=3;
           }else if(*u8>>3==0x1E){//4 BYTE
               bytes=4;
           }else if(*u8>>2==0x3E){//5 BYTE
               bytes=5;
           }else if(*u8>>1==0x7E){//6 BYTE
               bytes=6;
           }else{
               NGLOG_ERROR("CHAR:0x%02x error at %d %s",*u8,u8-str,str);
               NGLOG_DUMP("STR",str,len);
               nglSleep(1000);
           }
           for(int i=1;i<bytes;i++){
               if(u8[i]&0x80!=0x80){
                   NGLOG_ERROR("error at %d  bytes=%d",u8+i-str,bytes);
               }
               NGLOG_DUMP("STR",str,len);
               nglSleep(1000);
           }
           u8+=bytes;
      }
      if(u8err){
           NGLOG_DEBUG("Invalid UTF8:%s  \r\naddr%p %d errs",str,str,u8err);
           NGLOG_DUMP("INBUF",inbuf-1,len+1);
           NGLOG_DUMP("UTF8",str,len);
           //memcpy((void*)len,str,1000);
      }
      return u8err;
   }
};
void*CodeConverter::sohandle=nullptr;
_iconv_open CodeConverter::iconvopen=nullptr;
_iconv CodeConverter::iconv_conv=nullptr;
_iconv_close CodeConverter::iconvclose=nullptr;

INT Hex2BCD(INT x){
    INT bcd=0;
    for(int i=sizeof(INT)*8-4;i>=0;i-=4){
        bcd=bcd*10+((x>>i)&0x0F);
    }
    return bcd;
}

int ToUtf8(const char*buf,int len,char*utf){
    int utflen=255;
    char charset[16];//define chatset which used to convert to utf8
    int charpos=0;
    switch(buf[0]){
    case 0x01:strcpy(charset,"ISO8859-5");charpos=1;break;
    case 0x02:strcpy(charset,"ISO8859-6");charpos=1;break;
    case 0x03:strcpy(charset,"ISO8859-7");charpos=1;break;
    case 0x04:strcpy(charset,"ISO8859-8");charpos=1;break;
    case 0x05:strcpy(charset,"ISO8859-9");charpos=1;break;
    case 0x07:strcpy(charset,"ISO8859-11");charpos=1; break;//8859-11
    //case 0x08:break;//reseved for feature
    case 0x09:strcpy(charset,"ISO8859-13");charpos=1; break;//8859-13
    case 0x0A:strcpy(charset,"ISO8859-14");charpos=1; break;
    case 0x0B:strcpy(charset,"ISO8859-15");charpos=1; break;
    //case 0x0C-0x0F://reserved
    case 0x10:sprintf(charset,"ISO8859-%d",buf[2]);charpos=3;break;
    case 0x11:strcpy(charset,"ISO-10646"); charpos=1;  break;//ISO/IES 10646[16]
    case 0x12:strcpy(charset,"EUC-KR")   ; charpos=1;  break; //KSX1001-2004[44]
    case 0x13:strcpy(charset,"GB2312");    charpos=1;  break;//GB2312-1980[58]
    case 0x14:strcpy(charset,"BIG-5");     charpos=1;  break;//BIG5 subset of ISO/IES 10646[16]
    case 0x15://UTF8 encoding of ISO/IES 10646[16]
    default  :strcpy(charset,"ISO8859-1");charpos=0;   break;
    }
    CodeConverter cc(charset,"utf-8");
    len-=charpos;
    utflen=cc.convert((char*)(buf+charpos),len,utf,utflen);
    utf[utflen]=0;
    NGLOG_VERBOSE(">>>>ToUtf8 convert=%d charset=%s ",utflen,charset);
    return utflen;
}

void MJD2YMD(INT mjd,INT*y,INT*m,INT*d){
    INT Y,M,D,K;
    Y = ((float)mjd-15078.2f)/365.25f;
    M = ((float)mjd-14956.1f-int(365.25f*Y))/30.6001f;
    D = mjd-14956-int(365.25f*Y)-int(30.6001f*M);
    K=(14==M)||(15==M);
    Y+=K;
    M=M-1-K*12;
    if(y)*y=Y;//from 1900
    if(m)*m=M;//1-12
    if(d)*d=D;//1-31
    //weekday=(mjd+2)%7+1
}

void UTC2Tm(INT mjd,INT utc,NGL_TM*tm){
    INT y,m,d;
    BYTE b;
    MJD2YMD(mjd,&y,&m,&d);
    tm->uiYear=y;
    tm->uiMonth=m-1;
    tm->uiMonthDay=d;

    b=(utc>>16)&0xFF;
    tm->uiHour=10*(b>>4)+(b&0x0F);
    b=(utc>>8)&0xFF;
    tm->uiMin =10*(b>>4)+(b&0x0F);
    b=utc&0xFF;
    tm->uiSec =10*(b>>4)+(b&0x0F);
}

void UTC2Time(INT mjd,INT utc,NGL_TIME*t){
    NGL_TM tm;
    UTC2Tm(mjd,utc,&tm);
    nglTmToTime(&tm,t);
}

static UINT CRC32_Table[256];

static int InitCRC32(){
    #define CRC32_POLYNOMIAL 0x04C11DB7
    for (int i = 0; i< 256; i++){
        unsigned int coef32 = i << 24;
        for (int j=0; j<8; j++) {
            if (coef32 & 0x80000000)
                coef32 = ((coef32 << 1) ^ CRC32_POLYNOMIAL);
            else
                coef32 <<= 1;
        }
        CRC32_Table[i] = coef32;
    }
}

UINT GetCRC32(const BYTE*buffer, size_t size){
    unsigned int crc32 = 0xFFFFFFFF;
    unsigned int cntByte;
    if(CRC32_Table[2]==0)
       InitCRC32();
    for (cntByte = 0; cntByte < size; cntByte++){
        crc32 = (crc32 << 8 ) ^ CRC32_Table[((crc32 >> 24) ^ *buffer++) & 0xFF];
    }
    return crc32;
}

