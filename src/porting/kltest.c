#include<aui_dsc.h>
#include<aui_kl.h>
#include<stdio.h>
#include<string.h>


int main(int argc,char*argv[]){
   int rc;
   aui_attr_dsc dscattr;
   aui_hdl hdl_dsc;
   aui_dsc_init(NULL,NULL);

   memset(&dscattr,0,sizeof(dscattr));
   aui_dsc_open(&dscattr,&hdl_dsc);
   
}
