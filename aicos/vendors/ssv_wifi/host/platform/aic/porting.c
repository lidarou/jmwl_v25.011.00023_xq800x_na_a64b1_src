/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0da2f1
#define LOG_MODULE    AMOD_VENDOR

#define _PORTING_C_
#include <rtos.h>
#include <porting.h>
#include <ssv_lib.h>
#include <aic_gpio_pin.h>
#include <gpio_inc.h>
#include <aic_osa_ex.h>


#define APP_INCLUDE_WIFI_FW

/*=============console==================*/
ssv_type_u8 hal_getchar(void)
{
    ssv_type_u8 data=0;

    //data = getchar();
    return data;
}

#if 0
#define PRINT_BUF_SIZE                  512
static char print_buf[PRINT_BUF_SIZE];
static char print_buf_2[PRINT_BUF_SIZE+24];

void hal_print(char *fmt, ...)
{
    va_list v_list;
    ssv_type_s32 ret;
    va_start(v_list, fmt);
    ret = vsnprintf(print_buf, PRINT_BUF_SIZE, fmt, v_list);
	if(ret < 0)
	{
		return;
	}
    va_end(v_list);

    print_buf[PRINT_BUF_SIZE-1] = 0;
    print_buf_2[0] = 0;
    strcat(print_buf_2, "####SSV#### ");
    if ((strlen(print_buf)>2) && (((print_buf[0] == '\r') && (print_buf[1] == '\n'))||((print_buf[1] == '\r') && (print_buf[0] == '\n'))))
    {
        strcat(print_buf_2, print_buf+2);
    }
    else
    {
        strcat(print_buf_2, print_buf);
    }
    SCI_TraceLow("%s", print_buf_2); 
}
#endif

/*=============Memory==================*/
//extern void * gp_malloc(ssv_type_u32 size);                          // SDRAM allocation
OS_APIs void *OS_MemAlloc( ssv_type_u32 size )
{
    /**
        *  Platform dependent code. Please rewrite
        *  this piece of code for different system.
        */
   // void * ptr = SCI_Malloc(size);
      void * ptr = aic_malloc(size);//SCI_Malloc(size);
    //ptr = malloc(size);
    if (ptr) aos_memset(ptr,0,size);
    return ptr;
}

/**
 *  We do not recommend using OS_MemFree() API
 *  because we do not want to support memory
 *  management mechanism in embedded system.
 */
OS_APIs void __OS_MemFree( void *m )
{
    /**
        *  Platform depedent code. Please rewrite
        *  this piece of code for different system.
        */
    if (m) 
    {
        aic_free(m);
    }
}

void OS_MemCPY(void *pdest, const void *psrc, ssv_type_u32 size)
{
    if (size > 0)
    {
        aos_memcpy(pdest, psrc, size);
    }

}

void OS_MemSET(void *pdest, ssv_type_u8 byte, ssv_type_u32 size)
{
    aos_memset(pdest, byte, size);
}

OS_APIs void *OS_MemZAlloc( ssv_type_u32 size )
{
    /**
        *  Platform dependent code. Please rewrite
        *  this piece of code for different system.
        */
    void * ptr = NULL;
    ptr = aic_malloc(size);
    if (ptr) aos_memset(ptr,0,size);
    return ptr;
}

OS_APIs void *OS_MemReAlloc(void *ptr, ssv_type_u32 size)
{

    void * new = NULL;
    new = OS_MemAlloc(size);
    if (NULL==new){
        return NULL;
    }
    //ToDo: we don't know the original buffer's size
    OS_MemCPY(new,ptr,size);    
    __OS_MemFree(ptr);
    //LOG_PRINTF("[WARN]we don't know the original buffer's size\r\n");
    return new;
}
//=====================Platform LDO EN ping setting=======================
#undef SSV_LDO_EN_PIN

//void aic_gpio_output(GPIO_PIN_ID index, GPIO_MODE mode);
//u32 aic_gpio_level_get(GPIO_PIN_ID index);
//void aic_gpio_input(GPIO_PIN_ID index, GPIO_MODE mode, GPIO_CBFUN gpio_cb, void *param);
//int aic_gpio_request(GPIO_PIN_ID index);
//void aic_gpio_free(GPIO_PIN_ID index);

/* AT mode, do not disable gpio clock */
//void aic_gpio_at_mode(void);
void platform_ldo_en_pin_init(void)
{
    //GPIO_PullenSetup( pin,0);
   //GPIO_AON_9
   // GPIO_Enable(17);
   // GPIO_SetDirection(17, 1);

    //aic_gpio_request(GPIO_TOP_31);
 //   aic_gpio_output(32, GPIO_OUTPUT_LVL_HIGH);
   // aic_msleep(aic_msleep);
 //   aos_sleep(50);

}

void platform_ldo_en(ssv_type_bool en)
{
#if 0   
   if (!en)
   {
     //LOG_PRINTF("##sentton## WIFI CHIP GPIO_OUTPUT_LVL_LOW\n");
     aic_gpio_output(32, GPIO_OUTPUT_LVL_LOW);
   }else{
     //LOG_PRINTF ("##snetton## WIFI CHIP GPIO_OUTPUT_LVL_HIGH\n");
       aic_gpio_output(32, GPIO_OUTPUT_LVL_HIGH);
   }
   // sdhost_triger_scan(0);
#endif     
}

//=====================find fw to download=======================
#if 1
#include <ssv_hal.h>
ssv_type_bool platform_download_firmware(void)
{
    return ssv6xxx_download_fw((ssv_type_u8 *)SSV_HAL_FW_BIN,sizeof(SSV_HAL_FW_BIN));

#else  //this option is to set fw.bin as resource to open. please check resource_wifi_fw.s
void platform_download_firmware(void)
{
    ssv_type_u32 fw_size;
    extern ss_type_u8 *RES_WIFI_FW_START;
    extern ss_type_u8 *RES_WIFI_END;

    fw_size = ((u32)&RES_WIFI_END) - ((u32)&RES_WIFI_FW_START);
    LOG_PRINTF("fw_size=%d\r\n",fw_size);
    ssv6xxx_download_fw((u8 *)&RES_WIFI_FW_START,fw_size);//??? u8* bin
#endif
}
void platform_read_firmware(void *d,void *s,ssv_type_u32 len)
{
    OS_MemCPY(d,s,len);
}

void platform_udelay(ssv_type_u32 us_delay)
{
    if(us_delay<150000) //150ms
    {
        //osDelayUs(us_delay);
         OS_MsDelay(1);
    }
    else
    {
        OS_MsDelay(us_delay/1000);
    }
}   

