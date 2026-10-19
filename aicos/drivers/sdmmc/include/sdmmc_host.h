/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __HOST_H__
#define __HOST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <timer_inc.h>
#include <aic_osa.h>

#define VDD_165_195     (1 << 7)    /* VDD voltage 1.65 - 1.95 */
#define VDD_20_21       (1 << 8)    /* VDD voltage 2.0 ~ 2.1 */
#define VDD_21_22       (1 << 9)    /* VDD voltage 2.1 ~ 2.2 */
#define VDD_22_23       (1 << 10)   /* VDD voltage 2.2 ~ 2.3 */
#define VDD_23_24       (1 << 11)   /* VDD voltage 2.3 ~ 2.4 */
#define VDD_24_25       (1 << 12)   /* VDD voltage 2.4 ~ 2.5 */
#define VDD_25_26       (1 << 13)   /* VDD voltage 2.5 ~ 2.6 */
#define VDD_26_27       (1 << 14)   /* VDD voltage 2.6 ~ 2.7 */
#define VDD_27_28       (1 << 15)   /* VDD voltage 2.7 ~ 2.8 */
#define VDD_28_29       (1 << 16)   /* VDD voltage 2.8 ~ 2.9 */
#define VDD_29_30       (1 << 17)   /* VDD voltage 2.9 ~ 3.0 */
#define VDD_30_31       (1 << 18)   /* VDD voltage 3.0 ~ 3.1 */
#define VDD_31_32       (1 << 19)   /* VDD voltage 3.1 ~ 3.2 */
#define VDD_32_33       (1 << 20)   /* VDD voltage 3.2 ~ 3.3 */
#define VDD_33_34       (1 << 21)   /* VDD voltage 3.3 ~ 3.4 */
#define VDD_34_35       (1 << 22)   /* VDD voltage 3.4 ~ 3.5 */
#define VDD_35_36       (1 << 23)   /* VDD voltage 3.5 ~ 3.6 */

#define SDMMC_BUS_WIDTH_1       0
#define SDMMC_BUS_WIDTH_4       2
#define SDMMC_BUS_WIDTH_8       3


#define SDMMC_BUSWIDTH_4    (1 << 0)
#define SDMMC_BUSWIDTH_8    (1 << 1)
#define SDMMC_MUTBLKWRITE   (1 << 2)
#define SDMMC_HOST_IS_SPI   (1 << 3)
#define SDMMC_SUP_SDIO_IRQ  (1 << 4)    /* support signal pending SDIO IRQs */
#define SDMMC_SUP_HIGHSPEED (1 << 5)    /* support high speed */

//#define controller_is_spi(host) (host->flags & SDMMC_HOST_IS_SPI)
#define controller_is_spi(host) FALSE

/* runtime power managemanet */
typedef struct sdmmc_host_rpm_st {
    boolean lowpower;
    uint32_t rpm_checkcnt;
} sdmmc_host_rpm;

struct sdmmc_card_st;
typedef struct sdmmc_host_st {
    struct sdmmc_card_st *card;
    struct sdmmc_host_ops_st *ops;
    uint32_t freq_min;
    uint32_t freq_max;
    uint32_t valid_ocr; /* current valid OCR */
    uint32_t flags; /* define device capabilities */
    uint32_t max_seg_size;   /* maximum size of one dma segment */
    uint32_t max_dma_segs;   /* maximum number of dma segments in one request */
    uint32_t max_blk_size;   /* maximum block size */
    uint32_t max_blk_count;  /* maximum block count */
    uint32_t irq_num;

    uint32_t sdio_irq_num;
    aos_semaphore_ptr sdio_irq_sem;
    aos_thread_id sdio_irq_thread;

    void *private_data;
    void *power_io;
    void *power_card;
    aos_event_ptr sdmmc_event;
    aos_mutex_ptr host_lock;
    aos_mutex_ptr sdmmc_mutex;
    uint32_t   spi_use_crc;
} sdmmc_host;

struct sdmmc_host_st;
struct sdmmc_req_st;
typedef struct sdmmc_host_ops_st {
    void (*request)(struct sdmmc_host_st *host, struct sdmmc_req_st *req);
    int32_t (*get_card_status)(struct sdmmc_host_st *host);
    void (*enable_sdio_irq)(struct sdmmc_host_st *host, int32_t en);
} sdmmc_host_ops;

inline void sdmmc_delay_ms(uint32_t ms)
{
    aic_mdelay(ms);
}

#ifdef __cplusplus
}
#endif

#endif
