/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_hal_uart.h
* Description:
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef ABUP_HAL_UART
#define ABUP_HAL_UART
#include "abup_stdlib.h"
#if !defined(ABUP_BOOTLOADER)&&!defined(ABUP_PURE_TLV)&&!defined(ABUP_HTTP_5_0_CLIENT)
#include "abup_at_device.h"
#endif
#ifdef ABUP_TIMERS
#include "abup_timer.h"
#endif

#ifdef ABUP_TIMERS
abup_extern AbupTimerHandle_t AbupAtpTimer;
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG NULL
#endif
#if defined(ABUP_OPENFOTA)&&defined(__CCRL__)
abup_extern void abup_bl_main_printf(__far abup_char *buf);
#else
abup_extern void abup_bl_main_printf(abup_char *buf);
#endif
#ifndef ABUP_BL_PRINT_MAXLEN
#if (defined(WIN32) || defined(LINUX))&&defined(ABUP_DEBUG_MODE)
#define ABUP_BL_PRINT_MAXLEN
#elif defined(ABUP_DEBUG_MODE)
#define ABUP_BL_PRINT_MAXLEN 256
#endif
#endif
#ifdef ABUP_BL_PRINT_MAXLEN
#define abup_bl_debug_printf abup_kprintf
#ifndef peer_logout
#if defined(WIN32) || defined(LINUX)
#define peer_logout(CONTENT,...)  \
        do{ \
					abup_kprintf(CONTENT,##__VA_ARGS__); \
					abup_kprintf("\r\n"); \
        }while(0)
#else
#define peer_logout(CONTENT,...)  abup_kprintf(CONTENT"\r\n",##__VA_ARGS__)
#endif
#endif
#define abup_bl_debug_print(TAG,CONTENT,...)  \
        do{ \
					abup_kprintf(CONTENT,##__VA_ARGS__); \
        }while(0)
#elif !defined(ABUP_C51)
#define abup_bl_debug_printf(...)
#define abup_bl_debug_print(...)
#ifndef peer_logout
#define peer_logout(...)
#endif
#endif
#ifdef ABUP_UARTTX_MAXLEN
abup_extern abup_char ATBuf[ABUP_UARTTX_MAXLEN];
#endif
#if defined(ABUP_APP_DEBUG)&&(ABUP_APP_DEBUG == 1)
#define ABUP_USART_DEBUG_AT
#endif
#if !defined(ABUP_USART_DEBUG_BAUDRATE)
#define ABUP_USART_DEBUG_BAUDRATE 115200
#endif
#ifdef ABUP_UARTRX_MAXLEN
abup_extern abup_char AbupATRxBuf[ABUP_UARTRX_MAXLEN];
#endif
#ifdef ABUP_USART_DEBUG_AT
#define ABUP_UARTRX_DEBUGLEN                   255
abup_extern abup_uint8 AbupDebugRxBuf[ABUP_UARTRX_DEBUGLEN];
abup_extern abup_uint16 AbupDebugRxBufLen;
abup_extern abup_char *abup_get_AbupDebugRxBuf(void);
abup_extern abup_uint16 abup_get_AbupDebugRxBuf_len(void);
#endif
#if defined(ABUP_C51)||defined(__CCRL__)
#define abup_debug_printf abup_kprintf
#define abup_debug_override_function()
#elif defined(ABUP_DEBUG_MODE)
#define abup_debug_printf abup_kprintf
#if ((ABUP_DEBUG_UART_AT == 1)&&!defined(ABUP_BOOTLOADER)||(ABUP_APP_DEBUG == 1))&&defined(ABUP_UB)
#define abup_debug_override_function(...)
#else
#define abup_debug_override_function()  \
        do{ \
          static abup_bool first = abup_true; \
					if(first) \
					{ \
						ABUP_DEBUG(__func__,"function can be rewritten"); \
						first = abup_false; \
					} \
        }while(0)
#endif
#else
#define abup_debug_printf(...)
#define abup_debug_override_function(...)
#endif
#ifndef ABUP_TAG
#define ABUP_TAG  "Abup"
#endif
#ifndef ABUP_BOOTLOADER
#define ABUP_INFO(TAG,CONTENT,...)  \
        do{ \
          abup_kprintf("[%s] "CONTENT"\r\n",TAG, ##__VA_ARGS__); \
        }while(0)
#endif
#ifndef ABUP_C51
#define ABUP_DEBUG(TAG,CONTENT,...)  \
        do{ \
        	abup_debug_printf("\r\n[%s] "CONTENT"\r\n",TAG, ##__VA_ARGS__); \
        }while(0)
#endif
#if defined(WIN32) || defined(LINUX)
#include <stdio.h>
#define abup_kprintf printf
#else
abup_extern void abup_kprintf(const char *fmt, ...);
#endif
abup_extern abup_int abup_at_send_cmd(const abup_char *fmt, ...);
abup_extern void abup_at_send_data(abup_char *buf,abup_int16 len);
abup_extern abup_char *abup_get_ATBuf(void);
abup_extern abup_char *abup_get_AbupATRxBuf(void);
abup_extern abup_uint16 abup_get_ATBuf_len(void);
abup_extern abup_uint16 abup_get_AbupATRxBuf_len(void);
#endif


