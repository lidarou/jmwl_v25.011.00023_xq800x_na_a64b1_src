/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _OS_PORTING_H_
#define _OS_PORTING_H_

#include <ssv_types.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <aic_bitmap.h>
#include <aic_module.h>
#include <aic_osa_ex.h>
#include <nxd_bsd.h>
//#include "os_api.h"

typedef int             int32;
typedef unsigned int    uint32;
typedef short            int16;
typedef unsigned short    uint16;
typedef signed char        int8;
typedef unsigned char    uint8;

/* compile not support zero array */
#define NOT_SUPPORT_ZERO_ARRAY

/* sdio rx int, use card int trigger, pulldown/pullup */
#define SDIO_CARD_INT_TRIGGER

#define TMR_TASK_PRIORITY  (17)//(77)
#define ELOOP_TASK_PRIORITY (17)//(77)//(77)
#define MLME_TASK_PRIORITY (17)//(77)
#define CMD_ENG_PRIORITY   (17)//(77)
#define WIFI_RX_PRIORITY   (17)//(77)
#define WIFI_TX_PRIORITY   (16)//(76)

#define TCPIP_PRIORITY     (15)//(75)
#define DHCPD_PRIORITY     (15)//(75) 
#define NETAPP_PRIORITY    (15)//(75)

#define NETAPP_PRIORITY_1  (15)//(75)
#define NETAPP_PRIORITY_2  (15)//(75)
#define NETAPP_PRIORITY_3  (15)//(75)
#define NETAPP_PRIORITY_4  (15)//(75)
#define NETMGR_PRIORITY    (14)//(74)
#define SCONFIG_PRIORITY   (14)//(74)
#define TASK_END_PRIO      (13)//(73)

#define SSV_LDO_EN_PIN_ 1
//#define EXT_RX_INT 0 //sentton
/*============Console setting===================*/
//void hal_print(char *fmt, ...);

#define hal_print(fmt, ...)  alog_output(AMOD_NET, ALOG_LV_INFO, fmt, ##__VA_ARGS__)
//printf(fmt, ##__VA_ARGS__)

//SCI_TraceLow("####SSV#### "fmt, ##__VA_ARGS__) ##__VA_ARGS__

#define FFLUSH(x) fflush(x)

#define hal_putchar(ch) putchar(ch)
extern ssv_type_u8 hal_getchar(void);
#define ssv_inline __inline
#define inline __inline
#define Boolean unsigned char

/*============Compiler setting===================*/
#undef SSV_PACKED_STRUCT_BEGIN 
#define SSV_PACKED_STRUCT_BEGIN  
#define SSV_PACKED_STRUCT          
#define SSV_PACKED_STRUCT_END      
#define SSV_PACKED_STRUCT_STRUCT    __attribute__ ((packed))
#define SSV_PACKED_STRUCT_FIELD

#define UNION_PACKED
#define ALIGN_ARRAY(a) __attribute__((aligned(a)))
/*============SSV-DRV setting===================*/
#define	CONFIG_RX_POLL      0
#define INTERFACE "sdio"
#define SDRV_INCLUDE_SDIO   1

//#define INTERFACE "usb"
//#define SDRV_INCLUDE_USB   1

//#define SDIO_CARD_INT_TRIGGER

/*============Stack Size (unint: 16bytes)===================*/
#define TMR_TASK_STACK_SIZE  (128 * 8) //64
#define ELOOP_TASK_STACK_SIZE  1024
#define MLME_TASK_STACK_SIZE 0
//#define CMD_ENG_STACK_SIZE   (196 * 4) //128
#define CMD_ENG_STACK_SIZE   (240 * 16) //128
#define TCPIP_STACK_SIZE     0 //128
#define DHCPD_STACK_SIZE     0 //64
#define NETAPP1_STACK_SIZE    (128 * 8)
#define NETAPP2_STACK_SIZE    0
#define NETAPP3_STACK_SIZE    0
#define NETAPP4_STACK_SIZE    0
#define NETAPP5_STACK_SIZE    0
#define NETMGR_STACK_SIZE    (128 * 8)
#define CLI_TASK_STACK_SIZE  (128 * 8)
#define RX_ISR_STACK_SIZE    0
#define WIFI_RX_STACK_SIZE   (196 * 8) 
#define WIFI_TX_STACK_SIZE   (196 * 8) 
#define SCAN_TASK_STACK_SIZE  ( 128 * 8)
#define PING_THREAD_STACK_SIZE 0 //16 , ping thread doesn't enable now, I set staic size is zero to reduce data size.
#define SCONFIG_TASK_STACK_SIZE (128  * 8)

#define TOTAL_STACK_SIZE (TMR_TASK_STACK_SIZE+ \
                          ELOOP_TASK_STACK_SIZE+\
                          MLME_TASK_STACK_SIZE+ \
                          CMD_ENG_STACK_SIZE+ \
                          TCPIP_STACK_SIZE+ \
                          DHCPD_STACK_SIZE+ \
                          NETAPP1_STACK_SIZE+ \
						  NETAPP2_STACK_SIZE+ \
						  NETAPP3_STACK_SIZE+ \
						  NETAPP4_STACK_SIZE+ \
						  NETAPP5_STACK_SIZE+ \
                          NETMGR_STACK_SIZE+ \
                          CLI_TASK_STACK_SIZE+ \
						  RX_ISR_STACK_SIZE+ \
						  WIFI_RX_STACK_SIZE+ \
                          WIFI_TX_STACK_SIZE+ \
                          SCAN_TASK_STACK_SIZE+ \
                          PING_THREAD_STACK_SIZE+ \
                          SCONFIG_TASK_STACK_SIZE)
                          
/*============Memory========================*/
OS_APIs void *OS_MemAlloc( ssv_type_u32 size );
OS_APIs void __OS_MemFree( void *m );
OS_APIs void OS_MemSET(void *pdest, ssv_type_u8 byte, ssv_type_u32 size);
OS_APIs void OS_MemCPY(void *pdest, const void *psrc, ssv_type_u32 size);
OS_APIs void *OS_MemZAlloc( ssv_type_u32 size );
OS_APIs void *OS_MemReAlloc(void *ptr, ssv_type_u32 size);

/*=========================================*/
void platform_ldo_en_pin_init(void);
void platform_ldo_en(ssv_type_bool en);
void platform_udelay(ssv_type_u32 us_delay);
ssv_type_bool platform_download_firmware(void);
void platform_read_firmware(void *d,void *s,ssv_type_u32 len);
#endif
