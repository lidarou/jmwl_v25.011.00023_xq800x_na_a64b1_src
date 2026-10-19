/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is export uart phy interface.
 */

#ifndef __SERIAL_SERIAL_H__
#define __SERIAL_SERIAL_H__
#include "io.h"

void aic_uart_write(void *data, u8 *buf, u32 len);
u32 aic_uart_read(void *data, u8 *buf);
u8 aic_uart_getchar(void *data);
int aic_is_uart_recv_fifo_empty(void *data);
int aic_is_uart_send_fifo_empty(void *data);
void *aic_uart_port_open(int port);
void aic_uart_rx_trigger_level_register(void *dev, u32 val);
int aic_is_uart_rx_fifo_triger(void *dev);
void aic_uart_baud_set(void *dev, u32 baud);
void aic_uart_clr_status(void *data);
int aic_is_uart_recv_fifo_timeout(void *data);

#endif /*__SERIAL_SERIAL_H__*/
