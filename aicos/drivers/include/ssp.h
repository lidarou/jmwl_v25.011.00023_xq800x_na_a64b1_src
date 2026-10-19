/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

#ifndef __SSP_H__
#define __SSP_H__

#include <aic_type.h>
#include <aic_list.h>

#define SPI_CLK_12M5    (12500000)
#define SPI_CLK_25M     (25000000)
#define SPI_CLK_50M     (50000000)

enum spi_mode {
    SPI_MODE_0 = 0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3,
    SPI_MODE_MAX
};

enum bus_proto {
    PROTO_SPI = 0,
    PROTO_SSP,
    PROTO_MW,
    PROTO_I2S,
    PROTO_I2S_DSP,
    PROTO_MAX
};

enum work_mode {
    MASTER_MODE = 0,
    SLAVE_MODE,
    WORK_MODE_MAX
};

enum active_lvl {
    ACTIVE_LOW = 0,
    ACTIVE_HIGH,
    ACTIVE_LVL_MAX
};

#define SSP_NO_CS (0xFF)

typedef void (*slave_notify)(void *param);

struct ssp_cfg {
    enum spi_mode spi_mode;
    u32 work_clk;
    u8 cs;
    slave_notify slv_tx_notify;
    slave_notify slv_rx_notify;
    void *slv_param;
};

struct ssp_message {
    struct list_head head;
};

struct ssp_transfer {
    struct list_head node;
    u8 *tx_buf;
    u8 *rx_buf;
    u32 length;
};

static inline void ssp_message_init(struct ssp_message *msg)
{
    memset(msg, 0x0, sizeof(*msg));
    list_head_init(&(msg->head));
}

static inline void ssp_message_add_tail(struct ssp_transfer *xfer,
                                        struct ssp_message *msg)
{
    list_add_tail(&(xfer->node), &(msg->head));
}

int ssp_device_register(u32 *handle, char *name, char *bus_name,
                        struct ssp_cfg *cfg);
int ssp_configure(u32 handle, struct ssp_cfg *cfg);
int ssp_read(u32 handle, u8 *buf, u32 size);
int ssp_write(u32 handle, u8 *buf, u32 size);
int ssp_write_read(u32 handle, u8 *wbuf, u8 *rbuf, u32 size);
int ssp_write_then_read(u32 handle, u8 *wbuf, u32 wsize, u8 *rbuf, u32 rsize);
int ssp_write_then_write(u32 handle, u8 *wbuf, u32 wsize, u8 *wbuf1, u32 wsize1);
int ssp_sync(u32 handle, struct ssp_message *msg);

#endif /* __SSP_H__ */
