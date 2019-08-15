#include<aui_smc.h>
#include<va_types.h>
#include<va_sc.h>

BYTE PMTSection[];
int g_service_id;
void ProcessCommand(){
}
static void CardStateProc(int slot,unsigned int state){
   printf("Card %d state=%d\r\n",slot,state);
}
#define MAX_ATR_LN 33
int main(int argc ,char*argv[]){
   aui_hdl hdl;
   aui_smc_attr attr;
   aui_smc_param_t smc_param;

   int rc;
   unsigned char atr[MAX_ATR_LN];
   unsigned short atr_len = MAX_ATR_LN;

   aui_smc_init(NULL);
   memset(&attr,0,sizeof(attr));
   attr.ul_smc_id=0;
   attr.p_fn_smc_cb=CardStateProc;
   rc=aui_smc_open(&attr,&hdl);
   printf("aui_smc_open=%d hdl=%p\r\n",rc,hdl);
   memset(&smc_param,0,sizeof(smc_param));
   
   rc=aui_smc_reset(hdl,atr,&atr_len,1);
   printf("aui_smc_reset=%d\r\n",rc);

   VA_SC_ResetDone(0,atr,atr_len,0);
   printf("end of test.\r\n");
   return 0;
}
