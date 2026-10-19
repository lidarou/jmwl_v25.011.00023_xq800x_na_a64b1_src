/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_hal_uart.c
* Description: 串口驱动
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#include "abup_typedef.h"
#include "abup_stdlib.h"
#include "abup_hal_uart.h"

#if defined(ABUP_RTTHREAD)
abup_char uart_debug = 1;
abup_char uart_at = 2;
#elif defined(ABUP_RTT_LOG)
abup_char uart_debug = 1;
#endif
#ifdef ABUP_UARTTX_MAXLEN
#if defined(ABUP_TASK_SCHEDULER)
ABUP_ALIGN(1) abup_char abup_ts_buf[ABUP_UARTTX_MAXLEN]= {0};
#endif
ABUP_ALIGN(1) abup_char ATBuf[ABUP_UARTTX_MAXLEN]= {0};
#endif
#ifdef ABUP_UARTRX_MAXLEN
ABUP_ALIGN(1) abup_char AbupATRxBuf[ABUP_UARTRX_MAXLEN]= {0};
#endif
#if defined(ABUP_BL_PRINT_MAXLEN)
abup_char AbupPrintBuf[ABUP_BL_PRINT_MAXLEN]= {0};
#endif
#ifdef ABUP_USART_DEBUG_AT
ABUP_ALIGN(1) abup_uint8 AbupDebugRxBuf[ABUP_UARTRX_DEBUGLEN]= {0};
abup_uint16 AbupDebugRxBufLen = 0;
abup_char *abup_get_AbupDebugRxBuf(void)
{
    return (abup_char *)AbupDebugRxBuf;
}
abup_uint16 abup_get_AbupDebugRxBuf_len(void)
{
    return ABUP_UARTRX_DEBUGLEN;
}
#endif
abup_weak void abup_printf(abup_char *data,abup_int len)
{
    ABUP_UNUSED(data);
    ABUP_UNUSED(len);
}
abup_weak void abup_module_printf(abup_char *data,abup_int len)
{
    ABUP_UNUSED(data);
    ABUP_UNUSED(len);
}

abup_weak void abup_com_send_wait(abup_char *data)
{
    ABUP_UNUSED(data);
}
#ifdef ABUP_UARTTX_MAXLEN
abup_char *abup_get_ATBuf(void)
{
    abup_com_send_wait(ATBuf);
    return (abup_char *)ATBuf;
}
abup_uint16 abup_get_ATBuf_len(void)
{
    return ABUP_UARTTX_MAXLEN;
}
#endif
#ifdef ABUP_UARTRX_MAXLEN
abup_weak abup_char *abup_get_AbupATRxBuf(void)
{
    return (abup_char *)AbupATRxBuf;
}
abup_weak abup_uint16 abup_get_AbupATRxBuf_len(void)
{
    return ABUP_UARTRX_MAXLEN;
}
#endif
#if !defined(LINUX)&&!defined(WIN32)
void abup_kprintf(const char *fmt, ...)
{
#ifdef ABUP_DEBUG_MODE
    abup_int n;
    va_list args;
#ifdef ABUP_UARTTX_MAXLEN
#if defined(ABUP_TASK_SCHEDULER)
    abup_char *buf = abup_ts_buf;
#else
    abup_char *buf = abup_get_ATBuf();
#endif
    abup_int len = abup_get_ATBuf_len();
#elif defined(ABUP_BL_PRINT_MAXLEN)
    abup_char *buf = AbupPrintBuf;
    abup_int len = ABUP_BL_PRINT_MAXLEN;
#else
    abup_char data;
    abup_char *buf = &data;
    abup_int len = 1;
#endif

    va_start(args, fmt);
    n = abup_vsnprintf(buf, len, fmt, args);
    va_end(args);
    abup_printf(buf,n>len?len:n);
#elif defined(ABUP_UARTTX_MAXLEN)
    abup_int n;
    va_list args;
    abup_char *buf = abup_get_ATBuf();
    abup_int len = abup_get_ATBuf_len();
    va_start(args, fmt);
    n = abup_vsnprintf(buf, len, fmt, args);
    va_end(args);
    abup_printf(buf,n>len?len:n);
#elif defined(ABUP_BOOTLOADER)
    abup_int n;
    va_list args;
    abup_char buf[128];
    abup_int len = abup_sizeof(buf);
    va_start(args, fmt);
    n = abup_vsnprintf(buf, len, fmt, args);
    va_end(args);
    abup_printf(buf,n>len?len:n);
#endif
    ABUP_UNUSED(fmt);
}
#endif
void abup_bl_main_printf(abup_char *data)
{
    abup_printf(data,abup_strlen(data));
}

abup_int abup_at_send_cmd(const abup_char *fmt, ...)
{
    ABUP_UNUSED(fmt);
#ifdef ABUP_UARTTX_MAXLEN
    abup_int n;
    va_list args;
#if defined(ABUP_TASK_SCHEDULER)
    abup_char *buf = abup_ts_buf;
#else
    abup_char *buf = abup_get_ATBuf();
#endif
    abup_int len = abup_get_ATBuf_len();

    va_start(args, fmt);
    n = abup_vsnprintf(buf, len, fmt, args);
    va_end(args);
    abup_at_send_data(buf,n);
    return n;
#else
    return 0;
#endif
}
void abup_at_send_data(abup_char *data,abup_int16 len)
{
    if(len > 0)
    {
        abup_module_printf(data,len);
#ifdef ABUP_DEBUG_MODE
        abup_printf(data,len);
#endif
    }
#ifdef ABUP_TIMERS
#if !defined(ABUP_UB)
    AbupReStartTimer(AbupAtpTimer);
#endif
#endif
}
