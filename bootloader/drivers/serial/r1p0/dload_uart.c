/*****************************************************************************/
/*                                                                           */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is uart driver use fot download.
 */

#include "download/download.h"
#include "platform/platform.h"
#include "serial/serial.h"
#include "initcall.h"
#include "timer.h"

static uint32_t uart_read(void *data, uint8_t *buf, uint32_t len)
{
    uint32_t recv_cnt = 0;

    while (!aic_is_uart_recv_fifo_timeout(data))
        recv_cnt += aic_uart_read(data, buf + recv_cnt);

    aic_uart_clr_status(data);
    return recv_cnt;
}

static uint32_t uart_write(void *data, uint8_t *buf, uint32_t len)
{
    /*
    *sent whole TX buffer to pc, char by char.
    * make sure TX buffer is empty, whole buffer already sent.
    */
    aic_uart_write(data, buf, len);

    return 0;
}

static int uart_get_char_blocking(void *data)
{
    unsigned char ch;

    while (aic_is_uart_recv_fifo_empty(data))
        ;

    ch = aic_uart_getchar(data);
    aic_uart_clr_status(data);

    return ch;
}

static void uart_baudrate_set(void *data, uint32_t baudrate)
{
    while (!aic_is_uart_send_fifo_empty(data));

    aic_mdelay(2);

    aic_uart_baud_set(data, baudrate);
}

struct download uart0_download = {
    .write = uart_write,
    .read = uart_read,
    .get_char_blocking = uart_get_char_blocking,
    .baudrate_set = uart_baudrate_set,
};

struct download uart1_download = {
    .write = uart_write,
    .read = uart_read,
    .get_char_blocking = uart_get_char_blocking,
    .baudrate_set = uart_baudrate_set,
};

static int dload_uart_channel_init(void)
{
    uart0_download.data = aic_uart_port_open(3);
    download_driver_register(DL_UART0, &uart0_download);

    uart1_download.data = aic_uart_port_open(4);
    download_driver_register(DL_UART1, &uart1_download);
    return 0;
}
driver_init(dload_uart_channel_init);
