/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

#ifndef __SSPCORE_H__
#define __SSPCORE_H__

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_list.h>
#include <ssp.h>

#include "sspreg.h"

#define SSP_FIFO_DEPTH  (SSP_FIFO_SIZE * 4)

enum duplex_mode {
    FULL_DUPLEX = 0,
    HALF_DUPLEX,
    DUPLEX_MODE_MAX = 0xFF
};

enum data_width {
    DW_BYTE = 0,
    DW_HWORD,
    DW_WORD,
    DW_MAX
};

struct aic_ssp;

struct ssp_device {
    char *name;

    struct aic_ssp *bus;
    struct ssp_cfg cfg;

    GPIO_PIN_ID cs_gpio;
    enum active_lvl cs_active;

    struct list_head node;
};

struct ssp_ops {
    int (*transfer) (struct ssp_device *dev, struct ssp_message *msg);
    void (*ctrl_configure)(struct ssp_device *dev, struct ssp_cfg *cfg);
};

struct aic_ssp {
    struct ssp_device *owner;

    u32 dev_cnt;

    boolean dma_en;

    boolean slave;
    slave_notify slv_tx_notify;
    slave_notify slv_rx_notify;
    void *slv_param;

    boolean slp_backup_flag;
    struct ssp_reg slp_backup;

    struct ssp_ctrl *ctrl;
    struct ssp_ops *ops;

    aos_mutex_ptr mutex;

    aos_event_ptr event;

    struct list_head node;
    struct list_head head;
};

struct aic_ssp *ssp_get_handle(char *name);

#endif /* __SSPCORE_H__ */
