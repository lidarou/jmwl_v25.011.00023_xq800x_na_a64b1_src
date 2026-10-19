/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __SERIAL_INC_H__
#define __SERIAL_INC_H__

#include <aic_type.h>
#include <aic_bitmap.h>

typedef enum {
    UART_WSIZE1OR2 = 0,
    UART_WSIZE1,
    UART_WSIZE2,
    UART_WSIZE4,
} UART_WSIZE;

typedef enum {
    UART_INF0 = 0,
    UART_INF1,
    UART_INF2,
    UART_INF3,
    UART_INF4,
    UART_INF5,
} UART_INF;

typedef enum {
    UART_DCD_STATUS =   BIT_7,
    UART_RI_STATUS =    BIT_6,
    UART_DSR_STATUS =   BIT_5,
} UART_MODEM_STATUS;

typedef enum {
    UART_NBSTOP_1 =     0,
    UART_NBSTOP_1P5 =   1,
    UART_NBSTOP_2 =     2,
} UART_NBSTOP;

typedef enum {
    UART_PAR_EVEN =     0,
    UART_PAR_ODD =      1,
    UART_PAR_SPACE =    2,
    UART_PAR_MASK =     3,
    UART_PAR_NONE =     4,
} UART_PAR;

typedef enum {
    UART_CHRL_8 =       0,
    UART_CHRL_7 =       2,
    UART_CHRL_6 =       3,
} UART_CHRL;

/**
 * @UART_NORMAL_MODE: in which the UART operates as a standard UART.
 * @UART_AUTOMATIC_ECHO_MODE: in which the receiver data input is immediately
 *  transmitted through the transmitter output pin. No internal data can be
 *  transmitted from the UART but received data can be read by the processor.
 * @UART_LOCAL_LOOPBACK_MODE: in which the signal transmitted from the UART is
 *   connected to the receiver input. No data can be transmitted and no external
 *   data can be received.
 * @UART_REMOTE_LOOPBACK_MODE, in which the receiver input data is immediately
 *  transmitted through the transmitter output pin, ua_txd. No internal data can
 *  be transmitted from the UART and the UART cannot access received data.
**/
typedef enum {
    UART_NORMAL_MODE =          0,
    UART_AUTOMATIC_ECHO_MODE =  1,
    UART_LOCAL_LOOPBACK_MODE =  2,
    UART_REMOTE_LOOPBACK_MODE = 3,
} UART_LOOPBACK_MODE;

/**
 * UART_HOOK_FUNC - will be call it if produce DCD|RI|DSR Irq.
 *
 * Parameter:
 * @satus: UART_MODEM_STATUS.
 * @param: uart callback param.
 *
 * Return:
 *.
 */
typedef void (*UART_HOOK_FUNC)(u8 idx, u32 status, void *param);

/**
 * uart_open - open serial
 *  1.input parameter Com is aic_uart array index, aic_uart config at
 *    aicos\projects\xxx\config\serial\serial_config.h;
 *  2.use aic_uart[index] parameter to set serial.
 *
 * Parameter:
 * @com: input com is aic_uart array index.
 *
 * Return:
 *  return uart open handle if success, other return null.
 */
void *uart_open(int com);

/**
 * uart_close - close serial
 *
 * Parameter:
 * @handle: Input com operation handle.
 *
 * Return:
 *
 */
void uart_close(void *handle);

/**
 * uart_read - read data form serial
 *
 * Parameter:
 * @handle: input com operation handle.
 * @buffer: input read buffer ptr.
 * @size: input read size.
 *
 * Return:
 *  return actually read size.
 */
size_t uart_read(void *handle, void *buffer, size_t size);

/**
 * uart_read_dir - read data directory from serial fifo.
 *  1.not support dma.
 *  2.no wait, read data directory from serial fifo.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @buffer: input read buffer ptr.
 * @size: input read size.
 *
 * Return:
 *  return actually read size.
 */
size_t uart_read_dir(void *handle, void *buffer, size_t size);

/**
 * uart_write - wrtie data to serial
 *
 * Parameter:
 * @handle: input com operation handle.
 * @buffer: input write buffer ptr.
 * @size: input write size.
 *
 * Return:
 *  return actually write size.
 */
size_t uart_write(void *handle, const void *buffer, size_t size);

/**
 * uart_write_dir - write data directory to serial fifo, and not support dma.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @buffer: input write buffer ptr.
 * @size: input write size.
 *
 * Return:
 *  return actually write size.
 */
size_t uart_write_dir(void *handle, const void *buffer, size_t size);

/**
 * uart_set_baud_rate - set uart baudrate.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @baudrate: input set baudrate.
 *
 * Return:
 */
void uart_set_baud_rate(void *handle, int baudrate);

/**
 * uart_set_fctrl - set serial hardware flow control enable or not.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @enable: 1: enable, 0: disable.
 *
 * Return:
 */
void uart_set_fctrl(void *handle, boolean enable);

/**
 * uart_set_rts - set serial rts output.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @output: 1: output, 0: not output.
 *
 * Return:
 */
void uart_set_rts(void *handle, boolean output);

/**
 * uart_set_rx_triger_level - set serial rx trigger level.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: input rx trigger level.
 *
 * Return:
 */
void uart_set_rx_triger_level(void *handle, u32 val);

/**
 * uart_set_tx_triger_level - set serial tx trigger level.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: input tx trigger level.
 *
 * Return:
 */
void uart_set_tx_triger_level(void *handle, u32 val);

/**
 * uart_set_dtr - set serial dtr output.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @output: 1: output, 0: not output.
 *
 * Return:
 */
void uart_set_dtr(void *handle, boolean output);

/**
 * uart_wsize_set - set apb operation serial read and write size.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @wsize: 0: UART_WSIZE1OR2; 1: UART_WSIZE1; 2: UART_WSIZE2; 3: UART_WSIZE4.
 *
 * Return:
 */
void uart_wsize_set(void *handle, u8 wsize);

/**
 * uart_mode_set - set uart mode.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @mode: UART_LOOPBACK_MODE.
 *
 * Return:
 */
void uart_mode_set(void *handle, u8 mode);

/**
 * uart_nbstop_set - set uart number of stop bits.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @npstop: input stop bits.
 *
 * Return:
 */
void uart_nbstop_set(void *handle, u8 nbstop);

/**
 * uart_nbstop_get - get uart number of stop bits.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @npstop: output stop bits.
 *
 * Return:
 */
void uart_nbstop_get(void *handle, u8 *nbstop);

/**
 * uart_nbstop_set - set uart parity type.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @npstop: input parity type.
 *
 * Return:
 */
void uart_par_set(void *handle, u8 par);

/**
 * uart_par_get - get uart parity type.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @npstop: output parity type.
 *
 * Return:
 */
void uart_par_get(void *handle, u8 *par);

/**
 * uart_chrl_set - set uart character length.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @chrl: input character length.
 *
 * Return:
 */
void uart_chrl_set(void *handle, u8 chrl);

/**
 * uart_chrl_get - get uart character length.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @chrl: output character length.
 *
 * Return:
 */
void uart_chrl_get(void *handle, u8 *chrl);

/**
 * uart_iomux_sel - set serial hardware iomux.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @dst_inf: input hardware destination interface.
 *           0: UART_INF0
 *           1: UART_INF1
 *           2: UART_INF2
 *           3: UART_INF3
 *           4: UART_INF4
 *           5. UART_INF5
 *
 * Return:
 */
void uart_iomux_sel(void *handle, u8 dst_inf);

/**
 * uart_get_baud_rate - get serial baudrate.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output serial baudrate.
 *
 * Return:
 */
void uart_get_baud_rate(void *handle, u32 *val);

/**
 * uart_get_fctrl - get serial hardware flow control register value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output serial flow control register set value.
 *
 * Return:
 */
void uart_get_fctrl(void *handle, u32 *val);

/**
 * uart_get_dcd - get serial dcd value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output dcd value.
 *
 * Return:
 */
void uart_get_dcd(void *handle, u32 *val);

/**
 * uart_get_ri - get serial ri value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output ri value.
 *
 * Return:
 */
void uart_get_ri(void *handle, u32 *val);

/**
 * uart_get_dsr - get serial dst value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output dsr value.
 *
 * Return:
 */
void uart_get_dsr(void *handle, u32 *val);

/**
 * uart_get_cts - get serial cts value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output cts value.
 *
 * Return:
 */
void uart_get_cts(void *handle, u32 *val);

/**
 * uart_get_rx_triger - get serial rx trigger level.
 *
 * Parameter:
 * @handle: input com operation handle.
 *
 * Return:
 *  output serial rx trigger level
 */
int uart_get_rx_triger(void *handle);

/**
 * uart_get_tx_triger - get serial tx trigger level.
 *
 * Parameter:
 * @handle: input com operation handle.
 *
 * Return:
 *  output serial tx trigger level
 */
int uart_get_tx_triger(void *handle);

/**
 * uart_register_hook -
 *  register callback to uart, will be call it if produce DCD|RI|DSR Irq.
 *
 * Parameter:
 *  @func: input uart callback.
 *  @param: input callback param.
 *
 * Return:
 */
void uart_register_hook(UART_HOOK_FUNC func, void* param);

/**
 * uart_unregister_hook - unregister callback to uart.
 *
 * Parameter:
 *  @func: input uart callback.
 *
 * Return:
 */
void uart_unregister_hook(UART_HOOK_FUNC func);

/**
 * aic_uart_init - init the uart module.
 *
 * Parameter:
 *  @func: input uart callback.
 *
 * Return:
 */
int aic_uart_init(void);

#endif /* __SERIAL_INC_H__ */

