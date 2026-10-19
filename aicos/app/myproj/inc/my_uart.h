/********************************************************************
**版权所有         深圳市几米物联有限公司
**文件名称:        my_uart.h
**文件描述:        my_uart.c头文件声明
**当前版本:        V1.0
*********************************************************************/
#ifndef __MY_UART_H__
#define __MY_UART_H__

#define HAL_UART_1      0 // BLE uart
#define HAL_UART_2      1 // debug uart
#define HAL_UART_3      2 // GPS uart

#define MY_SHELL_PORT   HAL_UART_2
#define MY_GPS_PORT     HAL_UART_3
#define MY_BLE_PORT     HAL_UART_1


// 每次读事件发生时候临时读缓冲区
#define UART_BUF_MAX_LEN       512

#define MY_UART_BUFFER_SIZE    1024*2

typedef enum
{
    MY_UART_BAUD_9600   =  9600,
    MY_UART_BAUD_115200 =  115200,
    MY_UART_BAUD_921600 =  921600,
} MY_UART_BAUDRATE;

typedef uint32* uart_handle; // uart port handle

typedef struct
{
    uint32        port;    //串口的端口号
    uint32        baud;    //串口的波特率
    //module_type   mod;     //负责处理此UART read/write事件的模块
} MY_UART_ST_STRUCT;


void my_uart_init(MY_UART_ST_STRUCT *param);
void my_uart_close(int uart_id);

uint32 my_at_uart_read_data(char *buf, uint32 bufferlen);
uint32 my_gps_uart_read_data(char *buf, uint32 bufferlen);
uint32 my_uart_write_data(uint32 port, uint8 *buf, uint32 len);

#endif

