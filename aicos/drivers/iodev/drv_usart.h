/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    temp use drv_usart.h befor uart driver ready.
 */
#ifndef __DRV_USART_H__
#define __DRV_USART_H__
#include <aic_type.h>

/* temp use drv_usart.h befor uart driver ready. */
void *uart_open(int com);
void uart_close(void *handle);
size_t uart_read(void *handle, void *buffer, size_t size);
size_t uart_write(void *handle, const void *buffer, size_t size);
void uart_set_baud_rate(void *handle, int rate);

#endif /* __DRV_USART_H__ */
