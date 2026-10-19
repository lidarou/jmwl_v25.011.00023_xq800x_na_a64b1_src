/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_usart.h
* Description: 
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef ABUP_USART_H
#define ABUP_USART_H

#include "abup_typedef.h"
#include "usart.h"
#define LL_UART_Disable LL_USART_Disable
#define LL_UART_Enable LL_USART_Enable
#ifndef DEBUG_IRQn
#define DEBUG_IRQn
#endif
#ifndef MODULE_IRQn
#define MODULE_IRQn
#endif

#define _ABUP_UART(type,no) type##no
#define ABUP_UART(type,no) _ABUP_UART(type,no)
#ifndef _ABUP_UART_IRQn
#define _ABUP_UART_IRQn(type,no) type##no##_IRQn
#endif
#define ABUP_UART_IRQn(type,no) _ABUP_UART_IRQn(type,no)
#ifndef _ABUP_UART_IRQHandler
#define _ABUP_UART_IRQHandler(type,no) type##no##_IRQHandler
#endif
#define ABUP_UART_IRQHandler(type,no) _ABUP_UART_IRQHandler(type,no)
#define _ABUP_UART_CLK(type,no) LL_APB1_GRP1_PERIPH_##type##no
#define ABUP_UART_CLK(type,no) _ABUP_UART_CLK(type,no)
#define _ABUP_UART_TXRX_CLK(no) LL_AHB2_GRP1_PERIPH_GPIOC##no
#define ABUP_UART_TXRX_CLK(no) _ABUP_UART_TXRX_CLK(no)
#ifndef _ABUP_UART_Disable
#define _ABUP_UART_Disable(type) LL_##type##_Disable
#endif
#ifndef _ABUP_UART_Enable
#define _ABUP_UART_Enable(type) LL_##type##_Enable
#endif
#define ABUP_UART_Disable(type) _ABUP_UART_Disable(type)
#define ABUP_UART_Enable(type) _ABUP_UART_Enable(type)

#define ABUP_DEBUG_UART ABUP_UART(ABUP_DEBUG_UART_TYPE,ABUP_DEBUG_UART_NO)
#define ABUP_DEBUG_UART_IRQn ABUP_UART_IRQn(ABUP_DEBUG_UART_TYPE,ABUP_DEBUG_UART_NO)
#define ABUP_DEBUG_UART_IRQHandler ABUP_UART_IRQHandler(ABUP_DEBUG_UART_TYPE,ABUP_DEBUG_UART_NO)
#define ABUP_DEBUG_UART_CLK ABUP_UART_TXRX_CLK(ABUP_DEBUG_UART_TYPE,ABUP_DEBUG_UART_NO)

#define ABUP_MODULE_UART ABUP_UART(ABUP_MODULE_UART_TYPE,ABUP_MODULE_UART_NO)
#define ABUP_MODULE_UART_IRQn ABUP_UART_IRQn(ABUP_MODULE_UART_TYPE,ABUP_MODULE_UART_NO)
#define ABUP_MODULE_UART_IRQHandler ABUP_UART_IRQHandler(ABUP_MODULE_UART_TYPE,ABUP_MODULE_UART_NO)
#define ABUP_MODULE_UART_CLK ABUP_UART_TXRX_CLK(ABUP_MODULE_UART_TYPE,ABUP_MODULE_UART_NO)
#ifndef ABUP_DEBUG_UART_ENABLE
#define ABUP_DEBUG_UART_ENABLE 1
#endif
#ifndef ABUP_DEBUG_UART_AT
#define ABUP_DEBUG_UART_AT 0
#endif
#ifndef ABUP_DEBUG_UART_TYPE
#define ABUP_DEBUG_UART_TYPE USART
#endif
#ifndef ABUP_DEBUG_UART_NO
#define ABUP_DEBUG_UART_NO 2
#endif
#ifndef ABUP_DEBUG_UART_Init
#define ABUP_DEBUG_UART_Init MX_USART2_UART_Init
#endif
#ifndef ABUP_DEBUG_Disable
#define ABUP_DEBUG_Disable ABUP_UART_Disable(ABUP_DEBUG_UART_TYPE)
#endif
#ifndef ABUP_DEBUG_Enable
#define ABUP_DEBUG_Enable ABUP_UART_Enable(ABUP_DEBUG_UART_TYPE)
#endif

#ifndef ABUP_MODULE_UART_ENABLE
#define ABUP_MODULE_UART_ENABLE 1
#endif
#if defined(ABUP_OPENFOTA)||defined(ABUP_TLV_SERVER)
#ifndef ABUP_MODULE_UART_AT
#define ABUP_MODULE_UART_AT 1
#endif
#endif
#ifndef ABUP_MODULE_UART_TYPE
#define ABUP_MODULE_UART_TYPE USART
#endif
#ifndef ABUP_MODULE_UART_NO
#define ABUP_MODULE_UART_NO 1
#endif
#ifndef ABUP_MODULE_UART_Init
#define ABUP_MODULE_UART_Init MX_USART1_UART_Init
#endif
#ifndef ABUP_MODULE_Disable
#define ABUP_MODULE_Disable ABUP_UART_Disable(ABUP_MODULE_UART_TYPE)
#endif
#ifndef ABUP_MODULE_Enable
#define ABUP_MODULE_Enable ABUP_UART_Enable(ABUP_MODULE_UART_TYPE)
#endif

#if defined(ABUP_OPENFOTA)
#define ABUP_UART_TX_BUF_LEN 384//128
extern abup_char debug_tx[ABUP_UART_TX_BUF_LEN];
#elif !defined(ABUP_BOOTLOADER)
#include "abup_at_device.h"
extern char ATBuf[ABUP_UARTTX_MAXLEN];
#define ABUP_UART_TX_BUF_LEN abup_sizeof(ATBuf)
extern abup_char* debug_tx;
#else
extern char progress[128];
#define ABUP_UART_TX_BUF_LEN abup_sizeof(progress)
extern abup_char* debug_tx;
#endif
#define ABUP_UART_RX_BUF_LEN 384//0xFF
#ifndef ABUP_UART_MAX_TICK
#define ABUP_UART_MAX_TICK 20
#endif
#if (ABUP_DEBUG_UART_ENABLE == 1)&&!defined(ABUP_BOOTLOADER)||(ABUP_MODULE_UART_AT == 1)||!defined(ABUP_BOOTLOADER)
#define ABUP_UART_RX
#endif
typedef void (*abup_uart_cb)(void* ptr);
typedef enum
{
#if (ABUP_DEBUG_UART_ENABLE == 1)&&!defined(ABUP_BOOTLOADER)
    ABUP_UART_DEBUG,
#endif
#if (ABUP_MODULE_UART_AT == 1)||!defined(ABUP_BOOTLOADER)
    ABUP_UART_MODULE,
#endif
    ABUP_UART_MAX,
} ABUP_UART_TYPE;
typedef struct
{
    abup_uint32 tick;
    abup_char* rx;
    abup_char* tx;
    abup_uart_cb cb;
    abup_uint16 rxlen;
    abup_uint16 txlen;
    abup_uint16 rxsize;
    abup_uint16 txsize;
} ABUP_ALIGN(1) abup_uart_struct;
typedef struct
{
    abup_uint32 uart_no;
    abup_uart_struct* uart;
} ABUP_ALIGN(1) abup_uarts_struct;

abup_extern void AbupUartRecved(void);
#if (ABUP_DEBUG_UART_ENABLE == 1)
abup_extern void ABUP_DEBUG_UART_Enable(void);
#endif
#if (ABUP_MODULE_UART_ENABLE == 1)
abup_extern void ABUP_MODULE_UART_Enable(void);
#endif
abup_extern void abup_uart_received_data(void* ptr);
#if defined(__CCRL__)
abup_extern void abup_printf(__far abup_char *data,abup_int len);
abup_extern void abup_module_printf(__far abup_char * data,abup_int len);
#else
abup_extern void abup_printf(abup_char *data,abup_int len);
abup_extern void abup_module_printf(abup_char *data,abup_int len);
#endif
#if defined(ABUP_UART_RX)
abup_extern abup_uart_struct abup_uart[ABUP_UART_MAX];
#endif
#endif
