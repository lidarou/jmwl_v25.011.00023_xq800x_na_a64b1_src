/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <aic_type.h>
#include <aic_bitmap.h>
#include <slp_inc.h>
#include <serial_inc.h>

#define UART_RX_EVENT_FLAG          BIT_31
#define UART_TX_EVENT_FLAG          BIT_30
#define UART_TTRIG_EVENT_FLAG       BIT_10
#define UART_TIMEOUT_EVENT_FLAG     BIT_8
#define UART_TFUL_EVENT_FLAG        BIT_4
#define UART_TEMPTY_EVENT_FLAG      BIT_3
#define UART_RFUL_EVENT_FLAG        BIT_2
#define UART_REMPTY_EVENT_FLAG      BIT_1
#define UART_RTRIG_EVENT_FLAG       BIT_0

typedef enum {
    MUTEX_TX,
    MUTEX_RX,
    MUTEX_MAX,
} UART_MUTEX;

typedef enum {
    NONE_IRQ = 0x0,
    START_IRQ = 0x1,
    START_DMA = 0x2,
} UART_TX_RET;


struct dma_info {
    u32 hwid;
    struct uart_port *up;
};

enum dump_type{
    NO_DUMP_STEAM,
    CHAR_STREAM,
    HEX_STREAM,
};
struct uart_port {
    u8 index;
    aos_event_ptr event;
    aos_mutex_ptr mutex[MUTEX_MAX];
    char mutex_name[MUTEX_MAX][10];
    char *name;
    u32 inf;
    void *data;
    const void *tx_buffer;
    size_t tx_size;
    void *rx_buffer;
    size_t rx_size;
    size_t rx_tsize;
    struct dma_info tdma_info;
    struct dma_info rdma_info;
    boolean open;
    u32 slp_handle;
    boolean wakeup;
    u32 serial_status;
    aos_timer_ptr timer;
};

struct uart_core {
    struct uart_port *up;
    u32 port_num;
};

typedef struct _uart_cb {
    UART_HOOK_FUNC func;
    void *param;
    struct list_head node;
} UART_CB_T;

void uart_add_port(struct uart_port *up, u32 n);
void *uart_handle_get(int com);
UART_TX_RET aic_uart_start_tx(struct uart_port *up);
UART_TX_RET aic_uart_start_rx(struct uart_port *up);
int aic_uart_rx_fifo_empty(void *dev);
size_t aic_uart_write(void *dev, const char *s, u32 n);
size_t aic_uart_write_dir(void *dev, const char *s, u32 n);
size_t aic_uart_read(void *dev, char *s, u32 n);
void aic_uart_tx_trigger_level_set(void *dev, u32 val);
void aic_uart_rx_trigger_level_set(void *dev, u32 val);
void aic_uart_baud_set(void *dev, u32 baud);
void aic_uart_fctrl_set(void *dev, u32 val);
void aic_uart_rts_set(void *dev, u32 val);
void aic_uart_dtr_set(void *dev, u32 val);
void aic_uart_wsize_set(void *dev, u32 val);
void aic_uart_iomux_sel(void *dev, u32 dst_inf);
int aic_uart_rx_fifo_triger_get(void *dev);
int aic_uart_tx_fifo_triger_get(void *dev);
void aic_uart_baud_get(void *dev, u32 *val);
void aic_uart_fctrl_get(void *dev, u32 *val);
void aic_uart_dcd_get(void *dev, u32 *val);
void aic_uart_ri_get(void *dev, u32 *val);
void aic_uart_dsr_get(void *dev, u32 *val);
void aic_uart_cts_get(void *dev, u32 *val);
void aic_uart_wsize_get(void *dev, u32 *val);
void aic_uart_suspend(struct uart_port *up);
void aic_uart_resume(struct uart_port *up);
void aic_uart_open(struct uart_port *up);
void aic_uart_close(struct uart_port *up);
void aic_uart_register_hook(UART_HOOK_FUNC func, void* param);
void aic_uart_unregister_hook(UART_HOOK_FUNC func);
void aic_uart_mode_set(void *dev, u8 mode);
void aic_uart_nbstop_set(void *dev, u8 nbstop);
void aic_uart_nbstop_get(void *dev, u8 *nbstop);
void aic_uart_par_set(void *dev, u8 par);
void aic_uart_par_get(void *dev, u8 *par);
void aic_uart_chrl_set(void *dev, u8 chrl);
void aic_uart_chrl_get(void *dev, u8 *chrl);

#endif /* __SERIAL_INC_H__ */

