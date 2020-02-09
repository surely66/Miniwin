/******************************************************************************/
// INCLUDE
/******************************************************************************/
/*#include <sys_config.h>
#include <types.h>*/
/*#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/smc/smc.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libtsi/sie.h>
#include <api/libcas/mcas.h>
#include <api/libtsi/si_types.h>
#include <api/libchunk/chunk.h>
#include <api/libsi/sie_monitor.h>
#include <api/libc/fast_crc.h>
*/
#include <ngl_types.h>
#include "ngl_ca_platform.h"
//#include "ngl_ca_dbg.h"
#include <aui_dmx.h>
#include <aui_os.h>
#include <stdarg.h>

#ifdef  zhhou
typedef struct xFilterInfo
{
	struct get_section_param filter_section;
	struct restrict filter_restrict;
	INT8 filter_idx;
	DMXSectionCallback_F fn_cb;
	DWORD last_time;
	void *pvdata;
	DWORD dwReqID;
}FilterInfo_S;

typedef struct _Section_CB
{
	Section sectiondata;
	int filterIndex;
}Section_CB_S;

typedef struct _Section_DATA_C
{
	DWORD DataCRC;
	WORD DataHeader;
}Section_DATA_CRC;


#define MAX_FILTER_NUM (6)//
#define INAvailable_FILTER_ID (-1)
#define MAX_FILTER_SECTION_BUFFER (4096)

#define MAX_DATA_CRC_NUM (8)//

static FilterInfo_S g_FilterInfo[MAX_FILTER_NUM];
static DWORD g_dwdmxsemHandle = (DWORD)NULL;
static struct dmx_device *g_pDmxDev = NULL;
static struct smc_device *g_pSmcDev = NULL;
static SMCTaskCallback_F g_smcCB = NULL;
static BYTE g_dataBuf[MAX_FILTER_NUM][MAX_FILTER_SECTION_BUFFER];
static BYTE g_olddata[MAX_FILTER_NUM][MAX_FILTER_SECTION_BUFFER];//相同的section不送给库

static Section_DATA_CRC g_dataCRC[MAX_DATA_CRC_NUM] = {0};
static BYTE DataCRCNum =0;
static DWORD g_section_msgqueue_Emm = 0; 
static DWORD g_section_msgqueue_Ecm = 0; 

static void NGL_DMX_MUTEX_LOCK(void)
{
	DWORD result = -1;
	if(g_dwdmxsemHandle != 0)
	{
		result = NGLWaitForSemaphore(g_dwdmxsemHandle, 0xFFFFFFFF);
	}
	CAASSERT(result == 0);
}

static void NGL_DMX_MUTEX_UNLOCK(void)
{
	DWORD result = -1;
	if(g_dwdmxsemHandle != 0)
	{
		result = NGLReleaseSemaphore(g_dwdmxsemHandle, 1);
	}
	CAASSERT(result == 0);
}


/////printf--------------------start////////
static DWORD printfsem = 0;
//static BYTE g_debug_buf[1024];
static void LOCK_PRINTF(void)
{
	if(0 == printfsem)
	{
		 printfsem = NGLCreateSemaphore("x", 1, 1);
	}
	
	NGLWaitForSemaphore(printfsem, 0xFFFFFFFF);

}
static void UN_LOCK_PRINTF(void)
{
	NGLReleaseSemaphore(printfsem, 1);
}
#endif

void NGLPrintf(const char *format, ...)
{
	va_list Argument;

	//LOCK_PRINTF();

	BYTE g_debug_buf[1200];

	memset(g_debug_buf, 0, sizeof(g_debug_buf));

	va_start(Argument, format);
	vsprintf(g_debug_buf, format, Argument);
	va_end(Argument);

	libc_printf("%s", g_debug_buf);

	//UN_LOCK_PRINTF();
}
/////printf--------------------end////////

static unsigned long  NGL_ThreadProc(void*p1,void*p2)
{
        Task_F fnThreadEntry=(Task_F)p1;
        if(fnThreadEntry!=NULL)
            fnThreadEntry(p2);
}

DWORD NGLCreateThread(char* name, DWORD priority, DWORD dwStackSize, Task_F fnThreadEntry, void *pvArg, DWORD *pdwThreadId)
{
        aui_hdl hdl;
        aui_attr_task attr;
        strncpy(attr.sz_name,name,AUI_DEV_NAME_LEN);
        attr.ul_priority=priority;
        attr.ul_stack_size=dwStackSize;
        attr.p_fun_task_entry=NGL_ThreadProc;
        attr.para1=(unsigned long)fnThreadEntry;
        attr.para2=(unsigned long)pvArg;
        aui_os_task_create(&attr,&hdl);
	return (DWORD)hdl;
}

DWORD NGLDestroyThread(DWORD dwThreadID)
{
        aui_hdl hdl=(aui_hdl)dwThreadID;
        aui_os_task_delete(&hdl,0);	
	return 0;
}

DWORD NGLSuspendThread(DWORD dwThreadID)
{
	return -1;
}

DWORD NGLResumeThread(DWORD dwThreadID)
{
	CAASSERT(0);
	return -1;
}

DWORD NGLCreateSemaphore(char* name, int initcount, int maxcount)
{
	aui_hdl hdl;
        aui_attr_sem attr;
        strncpy(attr.sz_name,name,AUI_DEV_NAME_LEN);
        attr.ul_init_val=initcount;
        attr.ul_cnt=initcount;
        attr.ul_max_val=maxcount;
	aui_os_sem_create(&attr,&hdl);	
	return (DWORD)hdl;
}

DWORD NGLDestorySemaphore(DWORD semid)
{
        aui_hdl hdl=(aui_hdl)semid;
        aui_os_sem_delete(&hdl);
	return 0;
}

DWORD NGLWaitForSemaphore(DWORD semid, DWORD timeout)
{
       return aui_os_sem_wait((aui_hdl)semid,timeout)==AUI_RTN_SUCCESS;
}

DWORD NGLReleaseSemaphore(DWORD semid, DWORD howmany)
{
       aui_os_sem_release((aui_hdl)semid);
       CAASSERT(howmany==1);
       return 0;
}

DWORD NGLCreateMsgQueue(char* name, int howmany, int sizepermag, int value)
{
        aui_hdl hdl;
        aui_attr_msgq attr;
        attr.ul_buf_size=howmany*sizepermag;
        attr.ul_msg_size_max=sizepermag;
        aui_os_msgq_create(&attr,&hdl);
	return (DWORD)hdl;
}

DWORD NGLDestroyMsgQueue(DWORD msgid)
{
        aui_hdl hdl=(aui_hdl)msgid;
        aui_os_msgq_delete(&hdl);
	return 0;
}

DWORD NGLSendMsg(DWORD msgid, const void* pvmag, int msgsize, DWORD timeout)
{
        AUI_RTN_CODE rc=aui_os_msgq_snd((aui_hdl)msgid,(void*)pvmag,msgsize,timeout);
	return rc!=AUI_RTN_SUCCESS;
} 

DWORD NGLReceiveMsg(DWORD msgid, const void* pvmag, DWORD msgsize, DWORD timeout)
{
        unsigned long asize;
        aui_os_msgq_rcv((aui_hdl)msgid,(void*)pvmag,msgsize,&asize,timeout);
        CAASSERT(asize==msgsize);
	return 0;
} 

void NGLSleep(DWORD milliseconds)
{
        aui_os_task_sleep(milliseconds);	
}

#if zhhou
DWORD NGLNewStartTimer(char* name, TimerMode_E emode, DWORD interval, NEWTMR_CB fnCB)
{
	OSAL_T_CTIM 	t_dalm;
	ID				alarmID;

	t_dalm.callback = fnCB;
	if(EM_TIMERMODE_ONESHOT==emode)
	{
		t_dalm.type = TIMER_ALARM;
	}
	else
	{
		t_dalm.type = OSAL_TIMER_CYCLE;
	}
	t_dalm.time  = interval;
	
	alarmID = osal_timer_create(&t_dalm);
	if(OSAL_INVALID_ID != alarmID)
	{
		if(OSAL_TIMER_CYCLE == t_dalm.type)
		{
			osal_timer_activate(alarmID, 1);	
		}
		return alarmID;
		
	}
	else
	{
		CAASSERT(OSAL_INVALID_ID != alarmID);
		return 0;
	}
}

void NGLNewStopTimer(DWORD* pTimerID)
{
	ID timerID = *pTimerID;
	if(OSAL_INVALID_ID != timerID)
		osal_timer_delete(timerID);
	*pTimerID = OSAL_INVALID_ID;	
}

DWORD NGLCreateTimer(TimerMode_E eTimeMode, TimerCallback_F fnCallback, void* pvArg, DWORD* pdwTimerId)
{
	T_TIMER timer={0};
        timer.callback=fnCallback;
        timer.param=(UINT)pvArg;
        *pdwTimerId=os_create_timer(&timer);
	return 0;
}

DWORD NGLDestroyTimer(DWORD timerid)
{
	os_delete_timer(timerid);
	return 0;
}

DWORD NGLStartTimer(DWORD timerid, DWORD milliseconds)
{
	os_activate_timer(timerid,true);
	return 0;
}

DWORD NGLStopTimer(DWORD timerid)
{
	os_activate_timer(timerid,false);
	return 0;
}

#endif
DWORD NGLCreateEvent(char* name, DWORD flag)
{
        aui_hdl hdl;
        aui_attr_event attr;
        strncpy(attr.sz_name,name,AUI_DEV_NAME_LEN);
        attr.b_auto_reset=0;
        attr.b_initial_state=0;
        aui_os_event_create(&attr,&hdl);
	return (DWORD)hdl;
}


DWORD NGLDestroyEvent(DWORD eventid)
{
        aui_hdl hdl=(aui_hdl)eventid;
	aui_os_event_delete(&hdl);
	return 0;
}

DWORD NGLResetEvent(DWORD eventid)
{
	aui_os_event_clear((aui_hdl)eventid);
	return 0;
}

DWORD NGLSetEvent(DWORD eventid)
{
	aui_os_event_set((aui_hdl)eventid,0);
	return 0;
}

DWORD NGLWaitForSingleEvent(DWORD eventid, DWORD timeout)
{
        int rc=aui_os_event_wait((aui_hdl)eventid,timeout,0);
	return rc!=AUI_RTN_SUCCESS;
}

DWORD NGLMalloc(DWORD dwSize)
{
	return (DWORD)MALLOC(dwSize);
}

DWORD NGLMemset(void *dest, int c, UINT32  len)
{
	return (DWORD)MEMSET(dest, c, len);
}

DWORD NGLMemcpy(void *dest, const void *src, UINT32  len)
{
	return (DWORD)MEMCPY(dest, src, len);
}


DWORD NGLMemcmp(void *dest, const void *src, UINT32  len)
{
	return (DWORD)MEMCMP(dest, src, len);
}

DWORD NGLFree(void* paddr)
{
	FREE(paddr);
	paddr = NULL;
	return 1;
}

DWORD NGLGetTickCount(void)
{
	return 0;
}

DWORD NGLGetThreadId(void)
{
        aui_hdl hdl;
        aui_os_task_get_self_hdl(&hdl);
	return (DWORD)hdl;
}

#ifdef zhhou
//222222222222 flash api
DWORD NGLWriteFlash(DWORD dwStartAddr, BYTE *pData, DWORD lDataLen)
{
	INT32 ret;
	struct sto_device *sto_flash_dev;
	UINT32 param;
	UINT32 flash_cmd;

	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(sto_flash_dev == NULL)
	{
		CAASSERT(sto_flash_dev != NULL);
		return -1;
	}
	if (sto_open(sto_flash_dev) != CA_SUCCESS)
	{
		CAASSERT(0 && "sto_open error");
		return -1;
	}
	
	if(sto_flash_dev->totol_size <= 0x400000)
	{
		param = dwStartAddr << 10;
		param |= 0x10000 >> 10;
		flash_cmd = STO_DRIVER_SECTOR_ERASE;
	}
	else
	{
		UINT32 tmp_param[2];
		tmp_param[0] = dwStartAddr;
		tmp_param[1] = 0x10000 >> 10;
		param = (UINT32)tmp_param;
		flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
	}
	ret = sto_io_control(sto_flash_dev, flash_cmd, param);
	if(ret != CA_SUCCESS)
	{
		CAASSERT(0 && "sto_io_control error");
		return -1;
	}

	ret = sto_lseek(sto_flash_dev, (INT32)dwStartAddr, STO_LSEEK_SET);
	if (ret != (INT32)dwStartAddr)
	{
		CAASSERT(0 && "sto_lseek error");
		return -1;
	}
	ret = sto_write(sto_flash_dev, pData, lDataLen);
	if((UINT32) ret != lDataLen)
	{
		CAASSERT(0 && "sto_write error");
		return -1;
	}

	return 0;
}

DWORD NGLReadFlash(DWORD dwStartAddr, BYTE *pData, DWORD lDataLen)
{
	UINT32 key_table_addr = 0;
	UINT32 key_len = 0;
	struct sto_device *sto_flash_dev;
	INT32 ret = 0;

	if (((DWORD)NULL == dwStartAddr) || (NULL == pData) || (0 == lDataLen))
	{
		CAASSERT((DWORD)NULL == dwStartAddr);
		CAASSERT(NULL == pData);
		CAASSERT(0 == lDataLen);
		return -1;
	}

	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(sto_flash_dev == NULL)
	{
		CAASSERT(NULL == sto_flash_dev);
		return -1;
	}
	
	if (sto_open(sto_flash_dev) != CA_SUCCESS)
	{
		CAASSERT(0 && "sto_open flash error");
		return -1;
	}
	
	ret = sto_lseek(sto_flash_dev, (INT32)dwStartAddr, STO_LSEEK_SET);
	if (ret != (INT32)dwStartAddr)
	{
		CAASSERT(0 && "ret != (INT32)pStartAddr");		
		return -1;
	}
	
	if (sto_read(sto_flash_dev, pData, lDataLen) != (INT32)lDataLen)
	{
		CAASSERT(0 && "sto_read error");		
		return -1;
	}
	
	return 0;
}

//33333333333  descramble api

DWORD NGLDSMDescrambleSetOddKeyValue(DWORD dwDescrambleID, BYTE* pbOddKey, BYTE bOddLength)
{
	if(g_pDmxDev == NULL)
	{
		CAASSERT(g_pDmxDev != NULL);
		return -1;
	}

	int result = 0;
	
	result = dmx_cfg_cw(g_pDmxDev, DES_VIDEO, 3, (UINT32*)pbOddKey);
	//libc_printf(" odd DES_VIDEO--a = %d\n", result);
	
	result = dmx_cfg_cw(g_pDmxDev, DES_AUDIO, 3, (UINT32*)pbOddKey);
	//libc_printf(" odd DES_AUDIO--a = %d\n", result);
	
	result = dmx_cfg_cw(g_pDmxDev, DES_TTX, 3, (UINT32 *)pbOddKey);	
	//libc_printf(" odd DES_TTX--a = %d\n", result);
	
	result = dmx_cfg_cw(g_pDmxDev, DES_SUP, 3, (UINT32 *)pbOddKey);
	//libc_printf(" odd DES_SUP--a = %d\n", result);

	return 0;
}

DWORD NGLDSMDescrambleSetEvenKeyValue(DWORD  dwDescrambleID, BYTE *pbEvenKey, BYTE bEvenLength)
{
	if(g_pDmxDev == NULL)
	{
		CAASSERT(g_pDmxDev != NULL);
		return -1;
	}

	int result = 0;
	
	result = dmx_cfg_cw(g_pDmxDev, DES_VIDEO, 2, (UINT32*)pbEvenKey);	
	//libc_printf(" even DES_VIDEO--a = %d\n", result);
	
	result = dmx_cfg_cw(g_pDmxDev, DES_AUDIO, 2, (UINT32*)pbEvenKey);
	//libc_printf(" even DES_AUDIO--a = %d\n", result);
	
	result = dmx_cfg_cw(g_pDmxDev, DES_TTX, 2, (UINT32 *)pbEvenKey);	
	//libc_printf(" even DES_TTX--a = %d\n", result);
	
	result = dmx_cfg_cw(g_pDmxDev, DES_SUP, 2, (UINT32 *)pbEvenKey);
	//libc_printf(" even DES_SUP--a = %d\n", result);

	return 0;

}

void ali_setcw(unsigned char byKeyLen, BYTE* szOddKey, BYTE* szEvenKey, char *file)
{	
	NGLPrintf("[%s %d]from<%s>\n", __FUNCTION__, __LINE__, file);
	
	if((byKeyLen != 0) && (szEvenKey != NULL))
	{
		dmx_cfg_cw(g_pDmxDev, DES_VIDEO, 2, (UINT32*)szEvenKey);
		dmx_cfg_cw(g_pDmxDev, DES_AUDIO, 2, (UINT32*)szEvenKey);

		dmx_cfg_cw(g_pDmxDev, DES_TTX, 2, (UINT32 *)szEvenKey);
		dmx_cfg_cw(g_pDmxDev, DES_SUP, 2, (UINT32 *)szEvenKey);
	}

	if((byKeyLen != 0) && (szOddKey != NULL))
	{
		dmx_cfg_cw(g_pDmxDev, DES_VIDEO, 3, (UINT32*)szOddKey);
		dmx_cfg_cw(g_pDmxDev, DES_AUDIO, 3, (UINT32*)szOddKey);

		dmx_cfg_cw(g_pDmxDev, DES_TTX, 3, (UINT32 *)szOddKey);	
		dmx_cfg_cw(g_pDmxDev, DES_SUP, 3, (UINT32 *)szOddKey);
	}	
}



//4444444 smartcard api
DWORD NGLSMCConfig(DWORD index, int eProtocol, int eStandard, int eCaType)
{
	return 0;
}

void NGLSMCSetCardReaderResetDir(BOOL dir)
{
}
	
DWORD NGLSMCInit(void)
{
	//g_pSmcDev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	g_pSmcDev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
	if(g_pSmcDev == NULL)
	{
		CAASSERT(0);
		return -1;		
	}

	return 0;
}

static void smcTask(void* p)
{
	static DWORD oldexistok = -1;
	
	while(1)
	{
		DWORD newexistok = smc_card_exist(g_pSmcDev);
		
		if(newexistok != oldexistok)
		{
			if(0 == newexistok)
			{
				//NGLPrintf("[%s %d] scm in==\n", __FUNCTION__, __LINE__);
				g_smcCB(0, NGL_SMC_IN);
			}
			else if(1 == newexistok)
			{
				//NGLPrintf("[%s %d] scm out==\n", __FUNCTION__, __LINE__);
				g_smcCB(0, NGL_SMC_OUT);
			}
			else
			{
				NGLPrintf("[%s %d] unknow== \n", __FUNCTION__, __LINE__);
				CAASSERT(0 && "unknow status!");
			}

			oldexistok = newexistok;
		}

		NGLSleep(100);
	}
}

DWORD NGLSMCOpen(DWORD index, SMCTaskCallback_F fn)
{	
	g_smcCB = fn;
	DWORD dwTaskID = 0;

	if(NULL == g_pSmcDev)
	{
		CAASSERT(0);
		return -1;
	}

	NGLCreateThread("smc", TASKPRI_MIDDLE, 16*1024, smcTask, NULL, &dwTaskID);
	
	if(0 != smc_open(g_pSmcDev, NULL))
	{
		CAASSERT(0);
		return -1;		
	}
	
	return 0;
}

DWORD NGLSMCClose(DWORD index)
{
	if(NULL == g_pSmcDev)
	{
		CAASSERT(0);
		return -1;
	}
	
	if(0 != smc_close(g_pSmcDev))
	{
		CAASSERT(0);
		return -1;		
	}

	return 0;
}

DWORD NGLSMCDetectSmartCard(int index)
{
	if(NULL == g_pSmcDev)
	{
		CAASSERT(0);
		return -1;
	}

	if(smc_card_exist(g_pSmcDev) == 0)
	{
		return NGL_SMC_IN;
	}
	else
	{
		return NGL_SMC_OUT;
	}
}

DWORD NGLSMCResetSmartCardEx(DWORD dwCardIndex, BYTE* aucAtr, DWORD* pdwAtrLen, BOOL bColdReset)
{
	if(NULL == g_pSmcDev)
	{
		CAASSERT(0);
		return -1;
	}

	if(0 != smc_reset(g_pSmcDev, aucAtr, (UINT16*)pdwAtrLen))
	{
		CAASSERT(0);
		return -1;	
	}

	//for 2s card, 
	smc_io_control(g_pSmcDev, SMC_DRIVER_SET_WWT, 2500);
	
	return 0;
}

DWORD NGLSMCDataExchange(DWORD dwCardIndex, BYTE *pucWriteData, DWORD dwNumberToWrite, 
	BYTE *pucReadData, DWORD *pdwNumberRead, BYTE* pucStatusWord)
{
	DWORD read_buf = 256+10;
	DWORD actual_size = 0;
	
	if(NULL == g_pSmcDev)
	{
		CAASSERT(0);
		return -1;
	}

	
	if(0 != smc_iso_transfer(g_pSmcDev, (UINT8*)pucWriteData, (INT16)dwNumberToWrite, (UINT8*)pucReadData,*pdwNumberRead, (INT16*)&actual_size))
	{
		CAASSERT(0);
		return CS_SMC_FAILURE;	
	}

	if(actual_size < 2)
	{
		CAASSERT(0);
		return CS_SMC_FAILURE;
	}

	*pdwNumberRead = actual_size - 2;

	pucStatusWord[0] = pucReadData[actual_size-2];
	pucStatusWord[1] = pucReadData[actual_size-1];
	
	return CS_SMC_SUCCRSS;

}

DWORD NGLSMCSend(DWORD dwCardIndex, BYTE* pucWriteData, 
	DWORD dwNumberToWrite, DWORD *pdwNumberWrite, DWORD dwTimeout)
{
	CAASSERT(0);
	return -1;
}

DWORD NGLSMCReceive(DWORD dwCardIndex, BYTE* pucReadData, 
	DWORD dwNumberToRead, DWORD *pdwNumberRead, DWORD dwTimeout)
{
	CAASSERT(0);
	return -1;
}

static BYTE g_acc_section_data[40][4096];/**< static buffer for section data */
static int LLL = 0;/**< just for debug */

//55555 dmx api
static void filterTimeoutTask(void *p)
{
	int ii = 0;
	Section cb_data;

	memset(&cb_data, 0, sizeof(cb_data));

	while(1)
	{
		for(ii=0; ii<MAX_FILTER_NUM; ii++)
		{
			if(g_FilterInfo[ii].filter_idx != INAvailable_FILTER_ID)
			{
				if(NGLGetTickCount()-g_FilterInfo[ii].last_time > g_FilterInfo[ii].filter_section.wai_flg_dly)
				{
					g_FilterInfo[ii].fn_cb(SECTION_TIMEOUT, &cb_data, g_FilterInfo[ii].pvdata, (DWORD)&g_FilterInfo[ii]);

					//NGLPrintf("[%s %d]SECTION_TIMEOUT dwReqID:0x%x (%d)\n", __FUNCTION__, __LINE__, g_FilterInfo[ii].dwReqID, NGLGetTickCount());
				}
				
				if(NGLGetTickCount()-g_FilterInfo[ii].last_time > 3000)
				{
					DWORD ret = 0;

					NGL_DMX_MUTEX_LOCK();
					
					if(g_FilterInfo[ii].filter_idx != INAvailable_FILTER_ID)
					{
						NGLDebugPrintf(caDmx, EM_LEVEL_FATAL, "[%s %d]Emm timeout 3s, dwReqID:%d, ii:%d, LLL:%d\n",  __FUNCTION__, __LINE__, g_FilterInfo[ii].dwReqID, ii, LLL);

						g_FilterInfo[ii].last_time = NGLGetTickCount();
						
						dmx_io_control(g_pDmxDev, IO_ASYNC_CLOSE, g_FilterInfo[ii].filter_idx);
						ret = dmx_async_req_section(g_pDmxDev, &(g_FilterInfo[ii].filter_section), &(g_FilterInfo[ii].filter_idx));
						if(ret != 0)
						{
							NGLPrintf("[%s %d]dmx_async_req_section dwReqID:%d ii:%d idx:%d failed!!\n", __FUNCTION__, __LINE__, g_FilterInfo[ii].dwReqID, ii, g_FilterInfo[ii].filter_idx);
						}
						else
						{
							NGLDebugPrintf(caDmx, EM_LEVEL_FATAL, "[%s %d]dmx_async_req_section dwReqID:%d ii:%d idx:%d OK\n", __FUNCTION__, __LINE__, g_FilterInfo[ii].dwReqID, ii, g_FilterInfo[ii].filter_idx);
						}
					}
					
					NGL_DMX_MUTEX_UNLOCK();

				}
			}
		}

		NGLSleep(200);//第一次过滤条件不合适,这里延时太长,重新设置过滤变慢可能导致切台过慢 
	}
}

static void sectionCBtask_Emm(void *p)//emm 发送task
{
	Section_CB_S s_data;
	memset(&s_data, 0, sizeof(s_data));

	while(1)
	{
		if(NGLReceiveMsg(g_section_msgqueue_Emm,&s_data, sizeof(Section_CB_S), 0xFFFFFFFF) == 0)
		{
			//NGL_DMX_MUTEX_LOCK();

			if((s_data.filterIndex<MAX_FILTER_NUM) && (s_data.sectiondata.m_pucDataBuf!=0) && (s_data.sectiondata.m_nDataLen>0))
			{
				if((g_FilterInfo[s_data.filterIndex].filter_idx != INAvailable_FILTER_ID)
					&& (g_FilterInfo[s_data.filterIndex].dwReqID != 1)/*emm*/)
				{	
					NGLPrintf("[%s %d]Really get dada from:0x%x (tick:%d)\n", __FUNCTION__, __LINE__, g_FilterInfo[s_data.filterIndex].filter_idx, NGLGetTickCount());
					g_FilterInfo[s_data.filterIndex].fn_cb(SECTION_AVAIL, &s_data.sectiondata, g_FilterInfo[s_data.filterIndex].pvdata, (DWORD)&g_FilterInfo[s_data.filterIndex]);

				}
			}

			if(s_data.sectiondata.m_pucDataBuf != NULL)//保证不能内存泄漏
			{
				NGLFree(s_data.sectiondata.m_pucDataBuf);
				s_data.sectiondata.m_pucDataBuf = NULL;
			}
		
			//NGL_DMX_MUTEX_UNLOCK();
		}
	}
}

static void sectionCBtask_Ecm(void *p)//ecm 发送task
{
	Section_CB_S s_data;
	memset(&s_data, 0, sizeof(s_data));

	while(1)
	{
		if(NGLReceiveMsg(g_section_msgqueue_Ecm,&s_data, sizeof(s_data), 0xFFFFFFFF) == 0)
		{
			//NGL_DMX_MUTEX_LOCK();
			if((s_data.filterIndex<MAX_FILTER_NUM) && (s_data.sectiondata.m_pucDataBuf!=0) && (s_data.sectiondata.m_nDataLen>0))
			{
				if((g_FilterInfo[s_data.filterIndex].filter_idx != INAvailable_FILTER_ID)
					&& (g_FilterInfo[s_data.filterIndex].dwReqID == 1)/*ecm*/)
				{	
					//NGLPrintf("[%s %d]Really get dada from:0x%x (tick:%d)\n", __FUNCTION__, __LINE__, g_FilterInfo[s_data.filterIndex].filter_idx, NGLGetTickCount());
					g_FilterInfo[s_data.filterIndex].fn_cb(SECTION_AVAIL, &s_data.sectiondata, g_FilterInfo[s_data.filterIndex].pvdata, (DWORD)&g_FilterInfo[s_data.filterIndex]);

				}
			}

			if(s_data.sectiondata.m_pucDataBuf != NULL)//保证不能内存泄漏 
			{
				NGLFree(s_data.sectiondata.m_pucDataBuf);
				s_data.sectiondata.m_pucDataBuf = NULL;
			}

			//NGL_DMX_MUTEX_UNLOCK();
		}
	}
}

static DWORD casDmxInit(void)
{	
	if(g_pDmxDev ==NULL)/**< dmx dev init */
	{
		int ii = 0;
		DWORD dwTaskId = 0;
		
		g_pDmxDev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX,  0);//实时播放的dmx 
		if(g_pDmxDev == NULL)
		{
			CAASSERT(g_pDmxDev != NULL);
			return -1;
		}

		memset(g_FilterInfo, 0, sizeof(g_FilterInfo));

		for(ii=0; ii<MAX_FILTER_NUM; ii++)
		{
			g_FilterInfo[ii].filter_idx = INAvailable_FILTER_ID;
		}

		g_dwdmxsemHandle = NGLCreateSemaphore("dmx", 1, 0xFF);
		if(g_dwdmxsemHandle == 0)
		{
			CAASSERT(g_dwdmxsemHandle != 0);
			return -1;
		}

		g_section_msgqueue_Emm = NGLCreateMsgQueue("Emm", 32, sizeof(Section_CB_S), 0);

		if((DWORD)NULL == NGLCreateThread("Emm", OSAL_PRI_NORMAL, 8*1024, sectionCBtask_Emm, NULL, &dwTaskId))
		{
			CAASSERT(0 && "NGLCreateThread Emm error");
			return -1;
		}

		g_section_msgqueue_Ecm = NGLCreateMsgQueue("Ecm", 8, sizeof(Section_CB_S), 0);

		if((DWORD)NULL == NGLCreateThread("Ecm", OSAL_PRI_NORMAL, 8*1024, sectionCBtask_Ecm, NULL, &dwTaskId))
		{
			CAASSERT(0 && "NGLCreateThread Ecm error");
			return -1;
		}


		if((DWORD)NULL == NGLCreateThread("poll", OSAL_PRI_NORMAL, 8*1024, filterTimeoutTask, NULL, &dwTaskId))
		{
			CAASSERT(0 && "NGLCreateThread poll error");
			return -1;
		}
	}

	dmx_io_control(g_pDmxDev, DMX_BYPASS_CSA, 0);//pvr退出不能再播放加扰节目
	dmx_io_control(dev_get_by_id(HLD_DEV_TYPE_DMX,  2), DMX_BYPASS_CSA, 0);//pvr dmx csa
	
	return 0;
}

void changedmx2x(UINT8 prama)//For PVR dmx change!!
{
	int ii = 0;
	
	casDmxInit();

	NGL_DMX_MUTEX_LOCK();
	
	for(ii=0; ii<MAX_FILTER_NUM; ii++)
	{
		if(g_FilterInfo[ii].filter_idx !=  INAvailable_FILTER_ID)
		{
			dmx_io_control(g_pDmxDev, IO_ASYNC_CLOSE, g_FilterInfo[ii].filter_idx);
			NGLPrintf(" close idx:%d\n", g_FilterInfo[ii].filter_idx);
		}
		g_FilterInfo[ii].filter_idx = INAvailable_FILTER_ID;
	}

	g_pDmxDev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, prama);

	NGL_DMX_MUTEX_UNLOCK();
}

static void NGL_filter_get_sec_cb(struct get_section_param *param)
{
	FilterInfo_S *pfilter_info = (FilterInfo_S *)param;/**< 此地址转化有风险!!! */	
	Section_CB_S s_data;
	BYTE jj =0;
	DWORD DataCRC =0;
	//Section cb_data;
	int ii = 0;
	DWORD dwMsgQue = 0;
	
	for(ii=0; ii<MAX_FILTER_NUM; ii++)
	{
		if(&g_FilterInfo[ii] == pfilter_info)
		{
			if(MEMCMP(&g_olddata[ii][0], param->buff, param->sec_tbl_len) == 0)
			{
				return;
			}
			else
			{
				MEMCPY(&g_olddata[ii][0], param->buff, param->sec_tbl_len);
				
				for(jj = 0;jj< DataCRCNum;jj++){
					DataCRC = mg_table_driven_crc(0xFFFFFFFF, &g_olddata[ii][0],  param->sec_tbl_len);
					if(g_dataCRC[jj].DataHeader == g_olddata[ii][0])
					{
						if(DataCRC == g_dataCRC[jj].DataCRC)
							return;// data have send!!!
						else
						{
							g_dataCRC[jj].DataCRC = DataCRC;
							break;
						}
					}
						
				}
				//libc_printf("================>jj 0%d ,DataCRCNum %d\n ",jj,DataCRCNum);
				if(jj == DataCRCNum)
				{
					g_dataCRC[jj].DataHeader = g_olddata[ii][0];
					g_dataCRC[jj].DataCRC = mg_table_driven_crc(0xFFFFFFFF, &g_olddata[ii][0],  param->sec_tbl_len);
					//libc_printf("================>Get new data 0x%x.\n ",g_dataCRC[DataCRCNum].DataHeader);
					DataCRCNum ++;
				}
				if(DataCRCNum > MAX_DATA_CRC_NUM)
					DataCRCNum = 0;
			}
			
			g_FilterInfo[ii].last_time = NGLGetTickCount();
			
			if(g_FilterInfo[ii].dwReqID == 1)
			{
				dwMsgQue = g_section_msgqueue_Ecm;
			}
			else
			{
				dwMsgQue = g_section_msgqueue_Emm;
			}
			
			break;
		}
	}

	if(ii == MAX_FILTER_NUM)
	{
		CAASSERT(ii != MAX_FILTER_NUM);
		return;
	}
	
	if((param->buff == NULL) || (param->buff[0] == 0) || (pfilter_info->fn_cb == NULL))
	{
		CAASSERT(param->buff != NULL);
		CAASSERT(param->buff[0] == 0);
	}
	else
	{
		static int x = 0;
		s_data.filterIndex = ii;
		s_data.sectiondata.m_nDataLen = param->sec_tbl_len;

		s_data.sectiondata.m_pucDataBuf = (BYTE*)NGLMalloc(s_data.sectiondata.m_nDataLen);
		if(s_data.sectiondata.m_pucDataBuf == NULL)
		{
			NGLDebugPrintf(caDmx, EM_LEVEL_WARN, "Section callback, but can not malloc......\n");
			return;
		}

		
		memcpy(s_data.sectiondata.m_pucDataBuf, param->buff, param->sec_tbl_len); 
		if(NGLSendMsg(dwMsgQue, &s_data, sizeof(Section_CB_S), 0) != 0)
		{

			NGLFree(s_data.sectiondata.m_pucDataBuf);
			s_data.sectiondata.m_pucDataBuf = NULL;
			NGLDebugPrintf(caDmx, EM_LEVEL_WARN, "send data cb msg error dwReqID:%d!\n", pfilter_info->dwReqID);

		}
#if 0 //
		else
		{
			LLL++;
			x++;
			if(x == 40)
			{
				//NGLPrintf(" Is $40...\n");
				x = 0;
			}
		}
#endif
	}
}

DWORD NGLSectionRequest(RequestInfo *pRequestInfo, DMXSectionCallback_F fnSectionCallback)
{
	DWORD result = 0;
	int ii = 0;
	int jj = 0;
	DWORD dwHandle = (DWORD)NULL;
	DWORD ret = -1;

	result = casDmxInit();/**< IF success , only process one time */

	NGL_DMX_MUTEX_LOCK();

	#if 0
	if(result == 0)
	{
		for(ii=0; ii<MAX_FILTER_NUM; ii++)
		{
			if(g_FilterInfo[ii].filter_idx == INAvailable_FILTER_ID)
			{
				break;
			}
		}

		if(ii == MAX_FILTER_NUM)
		{
			result = -1;
			CAASSERT(0);
		}
	}
	#endif
	

	if(result == 0)
	{
		ii = pRequestInfo->dwReqID;//reqid作为ii
		g_FilterInfo[ii].filter_section.buff = &g_dataBuf[ii][0];
		g_FilterInfo[ii].filter_section.pid = (UINT16)pRequestInfo->dwPID;
		g_FilterInfo[ii].filter_section.buff_len = MAX_FILTER_SECTION_BUFFER;
		g_FilterInfo[ii].filter_section.crc_flag = 0;
		g_FilterInfo[ii].filter_section.mask_value = &(g_FilterInfo[ii].filter_restrict);

		g_FilterInfo[ii].filter_section.wai_flg_dly = pRequestInfo->lTimeout;
		g_FilterInfo[ii].filter_section.get_sec_cb = NGL_filter_get_sec_cb;//TODO;
		g_FilterInfo[ii].filter_section.continue_get_sec = 1;
		g_FilterInfo[ii].fn_cb = fnSectionCallback;
		g_FilterInfo[ii].pvdata = pRequestInfo->pvAppData;
		g_FilterInfo[ii].dwReqID = pRequestInfo->dwReqID;

		memset(&(g_FilterInfo[ii].filter_restrict), 0, sizeof(g_FilterInfo[ii].filter_restrict));
		for(jj = 0;jj<pRequestInfo->MMFilterCnt;jj++){
	
		g_FilterInfo[ii].filter_restrict.mask_len = pRequestInfo->MMFilter[jj].actuallen + 2;

		g_FilterInfo[ii].filter_restrict.value_num = 7;

		NGLDebugPrintf("caDmx", EM_LEVEL_INFO, "actuallen[%d]:0x%x",jj, pRequestInfo->MMFilter[jj].actuallen);
		///NGLDebugHex("caDmx", EM_LEVEL_INFO, "o-value", &pRequestInfo->MMFilter.match[0], 8);
		///NGLDebugHex("caDmx", EM_LEVEL_INFO, "o-mask", &pRequestInfo->MMFilter.mask[0], 8);
		/**< MASK */
		g_FilterInfo[ii].filter_restrict.mask[0] = pRequestInfo->MMFilter[jj].mask[0];
		g_FilterInfo[ii].filter_restrict.mask[1] = 0;
		g_FilterInfo[ii].filter_restrict.mask[2] = 0;/**<mask[1]  mask [2]section len.  lost*/
		memcpy(&(g_FilterInfo[ii].filter_restrict.mask[3]), &pRequestInfo->MMFilter[jj].mask[1], pRequestInfo->MMFilter[jj].actuallen-1);

		/**< MATCH */
		
		g_FilterInfo[ii].filter_restrict.value[jj][0] = pRequestInfo->MMFilter[jj].match[0];
		g_FilterInfo[ii].filter_restrict.value[jj][1] = 0;
		g_FilterInfo[ii].filter_restrict.value[jj][2] = 0;/**<value[1]  value [2]section len.  lost*/
		memcpy(&(g_FilterInfo[ii].filter_restrict.value[jj][3]), &pRequestInfo->MMFilter[jj].match[1], pRequestInfo->MMFilter[jj].actuallen-1);
		NGLDebugHex(caDmx, EM_LEVEL_DEBUG,  "d-value", &g_FilterInfo[ii].filter_restrict.value[jj][0], 8);
		NGLDebugHex(caDmx, EM_LEVEL_DEBUG,  "d-mask", &g_FilterInfo[ii].filter_restrict.mask[0], 8);
		}
		
		
		
		if(g_FilterInfo[ii].filter_idx != INAvailable_FILTER_ID)
		{
			CAASSERT(0);
			dmx_io_control(g_pDmxDev, IO_ASYNC_CLOSE, g_FilterInfo[ii].filter_idx);
			g_FilterInfo[ii].filter_idx = INAvailable_FILTER_ID;
		}
		
		g_FilterInfo[ii].last_time = NGLGetTickCount();
		ret = dmx_async_req_section(g_pDmxDev, &(g_FilterInfo[ii].filter_section), &(g_FilterInfo[ii].filter_idx));
		if(ret != 0)
		{
			NGLPrintf("[%s %d]dmx_async_req_section dwReqID:%d ii:%d idx:%d failed!!\n", __FUNCTION__, __LINE__, g_FilterInfo[ii].dwReqID, ii, g_FilterInfo[ii].filter_idx);
		}
		else
		{
			NGLDebugPrintf(caDmx, EM_LEVEL_FATAL, "[%s %d]dmx_async_req_section dwReqID:%d ii:%d idx:%d OK\n", __FUNCTION__, __LINE__, g_FilterInfo[ii].dwReqID, ii, g_FilterInfo[ii].filter_idx);
		}
	}

	if(result == 0)
	{
		dwHandle =  (DWORD)&g_FilterInfo[ii];
	}

	NGL_DMX_MUTEX_UNLOCK();
	
	return dwHandle;
}

BOOL NGLSectionCancel(DWORD dwHandle)
{
	int ii = 0;
	DWORD result = 0;
	BOOL rtn = FALSE;

	NGL_DMX_MUTEX_LOCK();
	
	for(ii=0; ii<MAX_FILTER_NUM; ii++)
	{
		if((DWORD)&g_FilterInfo[ii] == dwHandle)
		{
			break;
		}
	}

	if(ii >= MAX_FILTER_NUM)
	{
		CAASSERT(0);
		result = -1;
	}

	if(result==0)
	{
		if(g_FilterInfo[ii].filter_idx == INAvailable_FILTER_ID)
		{
			CAASSERT(0);
		}
		else
		{
			dmx_io_control(g_pDmxDev, IO_ASYNC_CLOSE, g_FilterInfo[ii].filter_idx);
		}
		
		//memset(&g_FilterInfo[ii], 0, sizeof(g_FilterInfo[ii]));
		memset(&g_olddata[ii][0], 0, MAX_FILTER_SECTION_BUFFER);
		g_FilterInfo[ii].filter_idx = INAvailable_FILTER_ID;
		
		rtn = TRUE;
	}

	NGL_DMX_MUTEX_UNLOCK();
	
	return rtn;
}

void NGLProcessCW(BYTE* pucInputKey, BYTE cLength, BYTE* pucOutputKey, BOOL bClearCW, int param)
{
	CAOSMemcpy(pucOutputKey, cLength, pucInputKey, cLength);
}

DWORD NGLGetSecuredType(SecuredType_EM *eType)
{
	*eType = SECURED_NACHIPSET_NASECURED;
	return 0;
}

DWORD NGLSecureInit(void)
{
	return 0;
}

DWORD NGLLoadCWPK(BYTE* pucKey, DWORD nLen, int nn)
{
	return 0;
}

#endif


