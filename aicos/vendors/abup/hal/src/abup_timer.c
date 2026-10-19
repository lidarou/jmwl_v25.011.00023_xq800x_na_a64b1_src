
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0bf3ef
#define LOG_MODULE    AMOD_VENDOR
#include "abup_typedef.h"
#include "abup_debug.h"
#include "abup_timer.h"
#include "abup_stdlib.h"

typedef struct timer_input_t
{
    abup_uint8 idx;
	abup_timer_callback func_ptr;

} TIMER_INPUT_MSG;


static ABUP_TIMER_STRUCT g_abup_timer[ABUP_TIMERID_MAXNUM]={0};
void abup_start_timer(abup_int32 delayms,abup_timer_callback func_ptr, ABUP_BOOL period)
{
	abup_uint8 idx = 0;
	abup_char name[16]={0};
	
	abup_memset(name,0,16);

	for (idx = 0; idx < ABUP_TIMERID_MAXNUM; idx++)
	{
		if (g_abup_timer[idx].func_ptr == func_ptr)
		{
			abup_DebugPrint("timer exist,restart before idx=%d", idx);
			if (aos_is_timer_created(g_abup_timer[idx].timerid))
			{
				abup_DebugPrint("timer exist, restart!");
				aos_change_timer(g_abup_timer[idx].timerid,delayms,period);
				return;
			}

			g_abup_timer[idx].timerid = 0;
			g_abup_timer[idx].func_ptr = NULL;
			g_abup_timer[idx].delay = 0;
		}
	}

	for (idx = 0; idx < ABUP_TIMERID_MAXNUM; idx++)
	{
		if(g_abup_timer[idx].func_ptr == NULL)
		{
			g_abup_timer[idx].func_ptr = func_ptr;
			break;
		}
	}
	abup_DebugPrint("%s: idx=%d", __func__, idx);

	if (idx >= ABUP_TIMERID_MAXNUM) 
		return;
	
	abup_snprintf(name, 16, "abup_timer_%d",idx);
	g_abup_timer[idx].delay = delayms;
	g_abup_timer[idx].timerid = aos_create_timer(name,
                                              (aos_expiration_func)g_abup_timer[idx].func_ptr,
                                              NULL,
                                              delayms,
                                              AOS_AUTO_ACTIVATE,
                                              period);
	abup_DebugPrint("%s: name=%s", __func__,name);
}


void abup_stop_timer(abup_timer_callback func_ptr)
{
	abup_uint8 idx = 0;

	abup_DebugPrint("%s: timer func: 0x%08x", __func__, (abup_uint32)func_ptr);

	for (idx = 0; idx < ABUP_TIMERID_MAXNUM; idx++)
	{
		if (g_abup_timer[idx].func_ptr == func_ptr)
		{
			break;
		}
	}

	if (idx >= ABUP_TIMERID_MAXNUM) 
		return;

	abup_DebugPrint("%s: timer idx:%d", __func__, idx);

	if (g_abup_timer[idx].timerid)
	{
		if (aos_is_timer_created(g_abup_timer[idx].timerid))
		{
			abup_DebugPrint("timer exist,delete idx=%d", idx);
			aos_deactivate_timer(g_abup_timer[idx].timerid);
			aos_delete_timer(g_abup_timer[idx].timerid);
		}
		g_abup_timer[idx].timerid = 0;
		g_abup_timer[idx].func_ptr = NULL;
		g_abup_timer[idx].delay = 0;
	}	
}

void abup_timer_handler(void *input)
{
	abup_uint8 idx;
	abup_DebugPrint("abup_timer_handler 1");
	
	TIMER_INPUT_MSG *msg = (TIMER_INPUT_MSG *)input;
	abup_timer_callback func_ptr = NULL;

	idx = msg->idx;
	abup_DebugPrint("abup_timer_handler idx=%d,msg->idx=%d",idx,msg->idx);



	if (idx >= ABUP_TIMERID_MAXNUM) return;

	func_ptr = g_abup_timer[idx].func_ptr;
	g_abup_timer[idx].timerid = 0;
	g_abup_timer[idx].func_ptr = NULL;
	g_abup_timer[idx].delay = 0;
	if (func_ptr) func_ptr();

	abup_DebugPrint("%s: timer idx:%d, send timer msg to abup", __func__, idx);
}

void abup_timer_cb(abup_uint32 timerid)
{		
	abup_DebugPrint("%s: timer idx:%d", __func__, timerid);
	
	if(timerid >= ABUP_TIMERID_MAXNUM) //abup_TIMERID_MAXNUM
		return; //timer is not found
	
	if(g_abup_timer[timerid].func_ptr)
		g_abup_timer[timerid].func_ptr();
}

void abup_timer1_cb(abup_uint32 timerid)
{
	#if 0
	static abup_uint32 timer_idx=0;
	abup_time_struct time;

	timer_idx++;

	abup_get_device_time(&time);
	
	abup_DebugPrint("abup curr date time1: %04d-%02d-%02d %02d:%02d:%02d", \
		time.nYear,time.nMonth,time.nDay, time.nHour, time.nMin,  time.nSec);

	abup_stop_timer(abup_timer1_cb);

	abup_DebugPrint("abup re start timer1: idx: %d", timer_idx);
	abup_start_timer(1000, abup_timer1_cb, ABUP_FALSE); 
	#endif
}

void abup_timer2_cb(abup_uint32 timerid)
{
	#if 0
	static abup_uint32 timer_idx=0;
	abup_time_struct time;

	timer_idx++;

	abup_get_device_time(&time);
	
	abup_DebugPrint("abup curr date time2: %04d-%02d-%02d %02d:%02d:%02d", \
		time.nYear,time.nMonth,time.nDay, time.nHour, time.nMin,  time.nSec);

	abup_stop_timer(abup_timer2_cb);

	abup_DebugPrint("abup re start timer2: idx: %d", timer_idx);
	abup_start_timer(1000, abup_timer2_cb, ABUP_FALSE); 
	#endif
}

void abup_timer3_cb(abup_uint32 timerid)
{
	#if 0
	static abup_uint32 timer_idx=0;
	abup_time_struct time;

	timer_idx++;

	abup_get_device_time(&time);
	
	abup_DebugPrint("abup curr date time3: %04d-%02d-%02d %02d:%02d:%02d", \
		time.nYear,time.nMonth,time.nDay, time.nHour, time.nMin,  time.nSec);

	abup_stop_timer(abup_timer3_cb);

	abup_DebugPrint("abup re start timer3: idx: %d", timer_idx);
	abup_start_timer(1000, abup_timer3_cb, ABUP_FALSE); 
	#endif
}


