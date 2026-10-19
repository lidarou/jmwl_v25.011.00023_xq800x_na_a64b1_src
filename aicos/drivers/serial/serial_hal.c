/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14d603
#define LOG_MODULE    AMOD_DRV

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_type.h>
#include <serial.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#include <aic_math.h>
#include <cache.h>
#include <aic_osa.h>
#include <aic_size.h>
#ifdef CONFIG_SUBSYS_AP
#include <connmgr_api.h>
#include <connmgr_inner.h>
#include <connmgr_cfg.h>
#endif
#include <aic_at_process_common.h>
#include <adbg_cfg_api.h>

#define UART_ACQUIRE_MUTEX(mutex_ptr) \
if (aos_is_user_thread_context()) { \
    u32 mutex_status; \
    AIC_ASSERT(NULL != mutex_ptr); \
    mutex_status = aos_get_mutex(mutex_ptr, AOS_WAIT_FOREVER); \
    AIC_ASSERT(AOS_SUCCESS == mutex_status); \
}

#define UART_RELEASE_MUTEX(mutex_ptr) \
if (aos_is_user_thread_context()) { \
    AIC_ASSERT(NULL != mutex_ptr); \
    aos_put_mutex(mutex_ptr); \
}

struct uart_core ucore;

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
size_t uart_read(void *handle, void *buffer, size_t size)
{
    struct uart_port *up = (struct uart_port *)handle;
    u32 actual_flags, event, status, wsize, uart_dp;
    UART_TX_RET ret;
    size_t n, rsize, i;
    u8 *ptr;

    UART_ACQUIRE_MUTEX(up->mutex[MUTEX_RX]);

UART_READ:
    if (!up->open) {
        UART_RELEASE_MUTEX(up->mutex[MUTEX_RX]);
        return 0;
    }

    event = UART_RX_EVENT_FLAG;
    up->rx_buffer = buffer;
    up->rx_size = size;
    up->rx_tsize = size;
    ptr = buffer;

    ret = aic_uart_start_rx(up);
    if (ret == START_DMA) {
        status = aos_get_event(up->event, event, AOS_AND_CLEAR, &actual_flags,
                               AOS_WAIT_FOREVER);
        AIC_ASSERT(!status);

        if (!up->open) {
            rsize = size - up->rx_size;
            UART_RELEASE_MUTEX(up->mutex[MUTEX_RX]);
            return rsize;
        }

        if (up->rx_size) {
            n = aic_uart_read(up->data, up->rx_buffer, up->rx_size);
            up->rx_buffer = (u8 *)up->rx_buffer + n;
            up->rx_size -= n;
        }
    }

    if ((size == (size - up->rx_size)) || (ret == START_DMA) ||
        (aic_uart_rx_fifo_empty(up->data) && (size - up->rx_size))) {
        rsize = size - up->rx_size;
        if (rsize == 0)
            goto UART_READ;
        up->rx_tsize = 0;
        UART_RELEASE_MUTEX(up->mutex[MUTEX_RX]);
        uart_dp = adbg_get_uartdump(up->index);
        if (uart_dp == CHAR_STREAM) {
            aicx_info(0x0a27c7, "dds", "uart_read size = %d, :%.*s\n", rsize,
                                                      rsize < 100 ? rsize : 100,
                                                      (u8 *)buffer);
        } else if (uart_dp == HEX_STREAM) {
            aicx_info(0x0cb982, "d", "uart_read size = %d\n", rsize);
            for (i = 0; i < (rsize < 100 ? rsize : 100); i ++)
                aicx_info(0x01733e, "x", "0x%x,", ((u8 *)buffer)[i]);
        }
        return rsize;
    }

    aic_uart_wsize_get(up->data, &wsize);
    if (aic_uart_rx_fifo_empty(up->data) ||
        ((wsize != 1) && !aic_uart_rx_fifo_triger_get(up->data))) {
        if (wsize != 1)
            event |= UART_RTRIG_EVENT_FLAG;
        status = aos_get_event(up->event, event, AOS_AND_CLEAR, &actual_flags,
                               AOS_WAIT_FOREVER);
        AIC_ASSERT(!status);
    }

    if (up->open)
        aic_uart_start_rx(up);

    rsize = size - up->rx_size;
    if (rsize == 0)
        goto UART_READ;

    up->rx_tsize = 0;
    UART_RELEASE_MUTEX(up->mutex[MUTEX_RX]);

    return rsize;
}

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
size_t uart_read_dir(void *handle, void *buffer, size_t size)
{
    struct uart_port *up = (struct uart_port *)handle;
    size_t n = 0, wsize, rsize;

    if (!up->open) {
        return 0;
    }

    aic_uart_wsize_get(up->data, &wsize);
    while (!aic_uart_rx_fifo_empty(up->data) && (n < size)) {
        if ((wsize != 1) && !aic_uart_rx_fifo_triger_get(up->data)) {
            continue;
        }
        rsize = aic_uart_read(up->data, buffer, size);
        n += rsize;
    }

    return n;
}

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
size_t uart_write(void *handle, const void *buffer, size_t size)
{
    struct uart_port *up = (struct uart_port *)handle;
    u32 actual_flags, status;
    UART_TX_RET ret;
    size_t rsize;

    aic_get_wakelock();

    UART_ACQUIRE_MUTEX(up->mutex[MUTEX_TX]);

    if (!up->open) {
        UART_RELEASE_MUTEX(up->mutex[MUTEX_TX]);
        aic_put_wakelock();
        return 0;
    }

    aos_get_event(up->event, UART_TX_EVENT_FLAG, AOS_AND_CLEAR,
                  &actual_flags, AOS_NO_WAIT);

    up->tx_buffer = buffer;
    up->tx_size = size;
    ret = aic_uart_start_tx(up);
    if ((ret == START_IRQ) || (ret == START_DMA)) {
        status = aos_get_event(up->event, UART_TX_EVENT_FLAG, AOS_AND_CLEAR,
                               &actual_flags, AOS_WAIT_FOREVER);
        AIC_ASSERT(!status);
    }

    rsize = size - up->tx_size;

    UART_RELEASE_MUTEX(up->mutex[MUTEX_TX]);

    aic_put_wakelock();

    return rsize;
}

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
size_t uart_write_dir(void *handle, const void *buffer, size_t size)
{
    struct uart_port *up = (struct uart_port *)handle;
    size_t rsize;

    if (!up->open) {
        return 0;
    }

    rsize = aic_uart_write_dir(up->data, buffer, size);

    return rsize;
}

/**
 * uart_set_baud_rate - set uart baudrate.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @baudrate: input set baudrate.
 *
 * Return:
 */
void uart_set_baud_rate(void *handle, int rate)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_baud_set(up->data, rate);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_set_fctrl - set serial hardware flow control enable or not.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @enable: 1: enable, 0: disable.
 *
 * Return:
 */
void uart_set_fctrl(void *handle, boolean enable)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_fctrl_set(up->data, enable);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_set_rts - set serial rts output.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @output: 1: output, 0: not output.
 *
 * Return:
 */
void uart_set_rts(void *handle, boolean output)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_rts_set(up->data, output);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_set_rx_triger_level - set serial rx trigger level.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: input rx trigger level.
 *
 * Return:
 */
void uart_set_rx_triger_level(void *handle, u32 val)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_rx_trigger_level_set(up->data, val);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_set_tx_triger_level - set serial tx trigger level.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: input tx trigger level.
 *
 * Return:
 */
void uart_set_tx_triger_level(void *handle, u32 val)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_tx_trigger_level_set(up->data, val);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_set_dtr - set serial dtr output.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: 1: output, 0: not output.
 *
 * Return:
 */
void uart_set_dtr(void *handle, boolean output)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_dtr_set(up->data, output);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_wsize_set - set apb operation serial read and write size.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @wsize: 0: UART_WSIZE1OR2; 1: UART_WSIZE1; 2: UART_WSIZE2; 3: UART_WSIZE4.
 *
 * Return:
 */
void uart_wsize_set(void *handle, u8 wsize)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_wsize_set(up->data, wsize);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_mode_set - set uart mode.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @mode: UART_LOOPBACK_MODE.
 *
 * Return:
 */
void uart_mode_set(void *handle, u8 mode)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_mode_set(up->data, mode);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_nbstop_set - set uart number of stop bits.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @npstop: input stop bits.
 *
 * Return:
 */
void uart_nbstop_set(void *handle, u8 nbstop)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_nbstop_set(up->data, nbstop);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_nbstop_get - get uart number of stop bits.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @npstop: output stop bits.
 *
 * Return:
 */
void uart_nbstop_get(void *handle, u8 *nbstop)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_nbstop_get(up->data, nbstop);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_nbstop_set - set uart parity type.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @npstop: input parity type.
 *
 * Return:
 */
void uart_par_set(void *handle, u8 par)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_par_set(up->data, par);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_par_get - get uart parity type.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @npstop: output parity type.
 *
 * Return:
 */
void uart_par_get(void *handle, u8 *par)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_par_get(up->data, par);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_chrl_set - set uart character length.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @chrl: input character length.
 *
 * Return:
 */
void uart_chrl_set(void *handle, u8 chrl)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_chrl_set(up->data, chrl);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_chrl_get - get uart character length.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @chrl: output character length.
 *
 * Return:
 */
void uart_chrl_get(void *handle, u8 *chrl)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_chrl_get(up->data, chrl);

    AOS_INTERRUPT_RESTORE
}

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
void uart_iomux_sel(void *handle, u8 dst_inf)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open) {
        aic_uart_iomux_sel(up->data, dst_inf);
        up->inf = dst_inf;
    }

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_get_baud_rate - get serial baudrate.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output serial baudrate.
 *
 * Return:
 */
void uart_get_baud_rate(void *handle, u32 *val)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_baud_get(up->data, val);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_get_fctrl - get serial hardware flow control register value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output serial flow control register set value.
 *
 * Return:
 */
void uart_get_fctrl(void *handle, u32 *val)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_fctrl_get(up->data, val);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_get_dcd - get serial dcd value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output dcd value.
 *
 * Return:
 */
void uart_get_dcd(void *handle, u32 *val)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_dcd_get(up->data, val);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_get_ri - get serial ri value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output ri value.
 *
 * Return:
 */
void uart_get_ri(void *handle, u32 *val)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_ri_get(up->data, val);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_get_dsr - get serial dst value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output dsr value.
 *
 * Return:
 */
void uart_get_dsr(void *handle, u32 *val)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_dsr_get(up->data, val);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_get_cts - get serial cts value.
 *
 * Parameter:
 * @handle: input com operation handle.
 * @val: output cts value.
 *
 * Return:
 */
void uart_get_cts(void *handle, u32 *val)
{
    struct uart_port *up = (struct uart_port *)handle;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        aic_uart_cts_get(up->data, val);

    AOS_INTERRUPT_RESTORE
}

/**
 * uart_get_rx_triger - get serial rx trigger level.
 *
 * Parameter:
 * @handle: input com operation handle.
 *
 * Return:
 *  output serial rx trigger level
 */
int uart_get_rx_triger(void *handle)
{
    struct uart_port *up = (struct uart_port *)handle;
    int rtriger = 0;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        rtriger = aic_uart_rx_fifo_triger_get(up->data);

    AOS_INTERRUPT_RESTORE

    return rtriger;
}

/**
 * uart_get_tx_triger - get serial tx trigger level.
 *
 * Parameter:
 * @handle: input com operation handle.
 *
 * Return:
 *  output serial tx trigger level
 */
int uart_get_tx_triger(void *handle)
{
    struct uart_port *up = (struct uart_port *)handle;
    int ttriger = 0;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE

    if (up->open)
        ttriger = aic_uart_tx_fifo_triger_get(up->data);

    AOS_INTERRUPT_RESTORE

    return ttriger;
}

/**
 * uart_creat_mutex - create uart mutex.
 *
 * Parameter:
 * @up: input uart port operation object.
 * @m: input mutex type.
 * @name: input mutex name.
 *
 * Return:
 *
 */
static void uart_creat_mutex(struct uart_port *up, UART_MUTEX m, char *name)
{
    strcpy(up->mutex_name[m], up->name);
    strcat(up->mutex_name[m], name);
    up->mutex[m] = aos_create_mutex(up->mutex_name[m]);
    AIC_ASSERT(up->mutex[m]);
}

u32 uart_dump_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *result_line0[2] = {"UART DUMP: (2, 1, 0)\n 0:NO DUMP STEAM, 1:CHAR STREAM, 2:HEX STREAM", "OK"};
    ulong *line_pointers;
    line_pointers = (ulong *)&result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);
    return AIC_SUCCESS;
}
u32 uart_dump_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *state_string[] = {"UART DUMP: NO DUMP STEAM(0)", "UART DUMP: CHAR STREAM(1)",
                          "UART DUMP: HEX STREAM(2)", "OK"};
    u16 paras_count, uart_index, line_num = 1;
    ulong *paras_arr, *line_pointers;
    u8 at_return_value[128] = {'\0'};
    struct uart_port *up;
    u8 *result_line0[2];
    u32 data_dump;
    int ret;
    line_pointers = (ulong *)&result_line0;
    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+UARTDUMP=", &paras_count);
    if (!paras_arr) {
       ret = AT_ERROR_INVALID_PARAS;
       goto err_out;
    }
    uart_index = atoi((u8 *) paras_arr[0]);
    if (uart_index > ucore.port_num) {
        ret = AT_ERROR_NO_SERIAL_CHANNEL;
        goto err_out;
    }
    up = &ucore.up[uart_index];
    if (!up->open) {
        ret = AT_ERROR_NO_SERIAL_CHANNEL;
        goto err_out;
    }
    line_num = 2;
     switch (paras_count) {
        case 2:
            data_dump = atoi((u8 *) paras_arr[1]);
            if (data_dump > HEX_STREAM) {
              ret = AT_ERROR_INVALID_PARAS;
              line_num = 1;
              goto err_out;
            }
            adbg_set_uartdump(uart_index, data_dump);
        case 1:
            data_dump = adbg_get_uartdump(uart_index);
            AIC_ASSERT(data_dump <= HEX_STREAM);
            result_line0[0] = state_string[data_dump];
            result_line0[1] = state_string[3];
            goto out;
        default:
            ret = AT_ERROR_INVALID_PARAS;
     }
err_out:
     snprintf(at_return_value, 50, "+CME ERROR: %d", ret);
     result_line0[0] = at_return_value;
out:
     aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, line_num);
     aic_free_paras_mem(paras_arr, paras_count);
     return AIC_SUCCESS;
}
static AT_CMD_HANDLE uart_at_handles[] = {
    {"AT+UARTDUMP", uart_dump_cmd, AIC_NULL, uart_dump_set_cmd, AIC_NULL},
    {AIC_NULL},
};
/**
 * uart_add_port - add port to ucore.
 *
 * Parameter:
 * @up: input uart port operation object.
 * @n: input uart number.
 *
 * Return:
 *
 */
void uart_add_port(struct uart_port *up, u32 n)
{
    u32 i;

    ucore.port_num = n;
    for (i = 0; i < n; i++) {
        uart_creat_mutex(&up[i], MUTEX_TX, "tx");
        uart_creat_mutex(&up[i], MUTEX_RX, "rx");
        up[i].event = aos_create_event(up[i].name);
        AIC_ASSERT(up[i].event);
    }
    ucore.up = up;
    aic_add_at_cmd_handle(uart_at_handles);
}

/**
 * uart_handle_get - get uart com handle.
 *
 * Parameter:
 * @com: input com is aic_uart array index.
 *
 * Return:
 *
 */
void *uart_handle_get(int com)
{
    if (ucore.port_num > com)
        return &ucore.up[com];
    else
        return NULL;
}

/**
 * uart_suspend - suspend uart before sleep.
 *
 * Parameter:
 * @handle: input uart operation handle.
 *
 * Return:
 *
 */
void uart_suspend(void *handle)
{
    struct uart_port *up = (struct uart_port *)handle;

    aic_uart_suspend(up);
}

/**
 * uart_resume - resume uart after wakeup.
 *
 * Parameter:
 * @handle: input uart operation handle.
 *
 * Return:
 *
 */
void uart_resume(void *handle)
{
    struct uart_port *up = (struct uart_port *)handle;

    aic_uart_resume(up);
}

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
void *uart_open(int com)
{
    struct uart_port *up;
    AOS_INTERRUPT_DECLARE;

    if (com >= ucore.port_num)
        return NULL;

    up = &ucore.up[com];

    AOS_INTERRUPT_DISABLE
    if (!up->open) {
        up->open = TRUE;
        AOS_INTERRUPT_RESTORE
    } else {
        AOS_INTERRUPT_RESTORE
        return NULL;
    }

    aic_uart_open(up);

#ifdef CONFIG_SUBSYS_AP
    u8 cnt = sizeof(s_log_dev_tbl) / sizeof(conn_dev_t);
    for (u8 i = 0; i < cnt; i++) {
        if ((s_log_dev_tbl[i].dev_type == AIC_IODEV_UART) &&
            (s_log_dev_tbl[i].dev_com == com) &&
            !up->slp_handle) {
            up->slp_handle = slp_register_sleep_hook_ex(uart_suspend, up,
                                                        uart_resume, up,
                                                        LV_HIGH);
        }
    }
#endif

    if (!up->slp_handle) {
        up->slp_handle = slp_register_sleep_hook_ex(uart_suspend, up,
                                                    uart_resume, up,
                                                    LV_MID);
    }

    return up;
}

/**
 * uart_close - close serial
 *
 * Parameter:
 * @handle: Input com operation handle.
 *
 * Return:
 *
 */
void uart_close(void *handle)
{
    struct uart_port *up;
    AOS_INTERRUPT_DECLARE;

    if (!handle)
        return;

    up = (struct uart_port *)handle;

    AOS_INTERRUPT_DISABLE
    if (up->open) {
        up->open = FALSE;
        AOS_INTERRUPT_RESTORE
    } else {
        AOS_INTERRUPT_RESTORE
        return;
    }

    slp_unregister_sleep_hook(up->slp_handle);
    up->slp_handle = 0;

    aic_uart_close(up);
}

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
void uart_register_hook(UART_HOOK_FUNC func, void* param)
{
    aic_uart_register_hook(func, param);
}

/**
 * uart_unregister_hook - unregister callback to uart.
 *
 * Parameter:
 *  @func: input uart callback.
 *
 * Return:
 */
void uart_unregister_hook(UART_HOOK_FUNC func)
{
    aic_uart_unregister_hook(func);
}

