/********************************************************************
**版权所有         深圳市几米物联有限公司
**文件名称:        my_uart.c
**文件描述:        设备平台uart数据收发处理模块
**当前版本:        V1.0
*********************************************************************/
#include "my_common.h"

uart_handle  g_uart_handle[6];  //uart_open返回对应的uart handle


uint32 my_at_uart_read_data(char *buf, uint32 bufferlen)
{
    uint32 cnt;

    cnt = uart_read(g_uart_handle[MY_SHELL_PORT], (void*)buf, (int)bufferlen);

    return cnt;
}


/************************************************************************
**函数名称:  my_gps_uart_read_data
**入口参数:  buf            ---      数据缓冲区
             bufferlen      ---      数据缓冲区的长度
**出口参数:  返回实际读取的数据个数
**函数功能:  读取GPS缓冲区中的数据信息
*************************************************************************/
uint32 my_gps_uart_read_data(char *buf, uint32 bufferlen)
{
    uint32  cnt;

    if (NULL == g_uart_handle[MY_GPS_PORT]) return 0;

    cnt = uart_read(g_uart_handle[MY_GPS_PORT], (uint8 *)buf, bufferlen);

    return cnt;
}


/************************************************************************
**函数名称:  my_uart_write_data
**入口参数:  port       ---     端口信息
             buf        ---     数据缓冲区信息
             len        ---     待写入数据的长度信息
**函数功能:  向串口输入数据信息
*************************************************************************/
uint32 my_uart_write_data(uint32 port, uint8 *buf, uint32 len)
{
    int bytesWrite = 0;

    bytesWrite = uart_write(g_uart_handle[port], (const void *)buf, len);

    if (bytesWrite < 0)
    {
        my_log_printf(1, "%s: return failed! port[%d]", __func__, port);
        return 0;
    }

    return (uint32)bytesWrite;
}



/************************************************************************
**函数名称:  my_uart_init
**入口参数:  param             ---        初始化数据信息
**出口参数:  无
**函数功能:  初始化指定的串口
*************************************************************************/
void my_uart_init(MY_UART_ST_STRUCT *param)
{
    uart_handle up_handle = NULL;

    up_handle = uart_open(param->port);
    if (NULL == up_handle)
    {
        my_log_printf(0, "%s: OPEN FAILED! PORT[%d]", __func__, param->port);
        return;
    }
    uart_set_baud_rate(up_handle, param->baud);
    g_uart_handle[param->port] = up_handle;
}

void my_uart_close(int uart_id)
{
    uart_handle up_handle = g_uart_handle[uart_id];

    if (NULL != up_handle)
    {
        uart_close(up_handle);
    }
}

