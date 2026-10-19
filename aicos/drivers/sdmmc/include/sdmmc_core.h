/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __SDMMC_CORE_H__
#define __SDMMC_CORE_H__

#include <sdmmc_host.h>
#include <sdmmc_card.h>
#include <sdmmc_cmd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DATA_DIR_WRITE  (1 << 0)
#define DATA_DIR_READ   (1 << 1)
#define DATA_STREAM (1 << 2)

typedef struct sdmmc_data_st {
    uint32_t  blksize;
    uint32_t  blks;
    uint32_t  *buf;
    int32_t  err;
    uint32_t  flags;
    unsigned int        bytes_xfered;
    void *stop;      /* stop command */
    void *mrq;       /* associated request */
    uint32_t  timeout_ns;
    uint32_t  timeout_clks;
} sdmmc_data;

/*rsponse types
 *bits:0~3
 */
#define RESP_MASK   (0xF)
#define RESP_NONE   (0)
#define RESP_R1     (1 << 0)
#define RESP_R1B    (2 << 0)
#define RESP_R2     (3 << 0)
#define RESP_R3     (4 << 0)
#define RESP_R4     (5 << 0)
#define RESP_R6     (6 << 0)
#define RESP_R7     (7 << 0)
#define RESP_R5     (8 << 0)    /*SDIO command response type*/
/*command types
 *bits:4~5
 */
#define CMD_MASK    (3 << 4)        /* command type */
#define CMD_AC      (0 << 4)
#define CMD_ADTC    (1 << 4)
#define CMD_BC      (2 << 4)
#define CMD_BCR     (3 << 4)

#define resp_type(cmd)  ((cmd)->flags & RESP_MASK)

/*spi rsponse types
 *bits:6~8
 */
#define RESP_SPI_MASK   (0x7 << 6)
#define RESP_SPI_R1 (1 << 6)
#define RESP_SPI_R1B    (2 << 6)
#define RESP_SPI_R2 (3 << 6)
#define RESP_SPI_R3 (4 << 6)
#define RESP_SPI_R4 (5 << 6)
#define RESP_SPI_R5 (6 << 6)
#define RESP_SPI_R7 (7 << 6)

#define spi_resp_type(cmd)  ((cmd)->flags & RESP_SPI_MASK)
/*
 * These are the command types.
 */
#define cmd_type(cmd)   ((cmd)->flags & CMD_MASK)

typedef struct sdmmc_cmd_st {
    uint32_t  cmd_code;
    uint32_t  arg;
    uint32_t  resp[4];
    uint32_t  flags;
    int32_t  retries;    /* max number of retries */
    int32_t  err;
    sdmmc_data *data;
    void *mrq;       /* associated request */
} sdmmc_cmd;

typedef struct sdmmc_req_st {
    sdmmc_data  *data;
    sdmmc_cmd   *cmd;
    sdmmc_cmd   *stop;
} sdmmc_req;

/*the following is response bit*/
#define R1_OUT_OF_RANGE     (1 << 31)   /* er, c */
#define R1_ADDRESS_ERROR    (1 << 30)   /* erx, c */
#define R1_BLOCK_LEN_ERROR  (1 << 29)   /* er, c */
#define R1_ERASE_SEQ_ERROR      (1 << 28)   /* er, c */
#define R1_ERASE_PARAM      (1 << 27)   /* ex, c */
#define R1_WP_VIOLATION     (1 << 26)   /* erx, c */
#define R1_CARD_IS_LOCKED   (1 << 25)   /* sx, a */
#define R1_LOCK_UNLOCK_FAILED   (1 << 24)   /* erx, c */
#define R1_COM_CRC_ERROR    (1 << 23)   /* er, b */
#define R1_ILLEGAL_COMMAND  (1 << 22)   /* er, b */
#define R1_CARD_ECC_FAILED  (1 << 21)   /* ex, c */
#define R1_CC_ERROR     (1 << 20)   /* erx, c */
#define R1_ERROR        (1 << 19)   /* erx, c */
#define R1_UNDERRUN     (1 << 18)   /* ex, c */
#define R1_OVERRUN      (1 << 17)   /* ex, c */
#define R1_CID_CSD_OVERWRITE    (1 << 16)   /* erx, c, CID/CSD overwrite */
#define R1_WP_ERASE_SKIP    (1 << 15)   /* sx, c */
#define R1_CARD_ECC_DISABLED    (1 << 14)   /* sx, a */
#define R1_ERASE_RESET      (1 << 13)   /* sr, c */
#define R1_STATUS(x)            (x & 0xFFFFE000)
#define R1_CURRENT_STATE(x) ((x & 0x00001E00) >> 9) /* sx, b (4 bits) */
#define R1_READY_FOR_DATA   (1 << 8)    /* sx, a */
#define R1_APP_CMD      (1 << 5)    /* sr, c */

#define CARD_BUSY   0x80000000  /* Card Power up status bit */

/* R5 response bits */
#define R5_COM_CRC_ERROR    (1 << 15)
#define R5_ILLEGAL_COMMAND  (1 << 14)
#define R5_ERROR            (1 << 11)
#define R5_FUNCTION_NUMBER  (1 << 9)
#define R5_OUT_OF_RANGE     (1 << 8)
#define R5_STATUS(x)        (x & 0xCB00)
#define R5_IO_CURRENT_STATE(x)  ((x & 0x3000) >> 12)

int sdmmc_wait_cd_changed(int32_t timeout);
void sdmmc_host_lock(sdmmc_host *host);
void sdmmc_host_unlock(sdmmc_host *host);
void sdmmc_req_complete(sdmmc_host *host);
void sdmmc_send_request(sdmmc_host *host, sdmmc_req *req);
int32_t sdmmc_send_cmd(sdmmc_host *host, sdmmc_cmd *cmd, int retries);
int32_t sdmmc_go_idle(sdmmc_host *host);
int32_t sdmmc_spi_read_ocr(sdmmc_host *host, int32_t high_capacity, uint32_t *ocr);
int32_t sdmmc_all_get_cid(sdmmc_host *host, uint32_t *cid);
int32_t sdmmc_get_cid(sdmmc_host *host, uint32_t *cid);
int32_t sdmmc_get_csd(sdmmc_card *card, uint32_t *csd);
int32_t sdmmc_select_card(sdmmc_card *card);
int32_t sdmmc_deselect_cards(sdmmc_card *host);
int32_t sdmmc_spi_use_crc(sdmmc_host *host, int32_t use_crc);
void sdmmc_set_chip_select(sdmmc_host *host, int32_t mode);
void sdmmc_set_clock(sdmmc_host *host, uint32_t clk);
void sdmmc_set_bus_mode(sdmmc_host *host, uint32_t mode);
void sdmmc_set_bus_width(sdmmc_host *host, uint32_t width);
void sdmmc_set_data_timeout(sdmmc_data *data, const sdmmc_card *card);
uint32_t sdmmc_select_voltage(sdmmc_host *host, uint32_t ocr);
void sdmmc_change(sdmmc_host *host);
void sdmmc_detect(void *param);
sdmmc_host *sdmmc_alloc_host(void);
void sdmmc_free_host(sdmmc_host *host);
int sdmmc_blk_init(void);
int32_t sdmmc_blk_probe(sdmmc_card *card);
void sdmmc_blk_remove(sdmmc_card *card);
void sdmmc_set_dfi(sdmmc_host *host, uint32_t sdcard_speed, uint32_t uhsi_mode);
int sdmmc_core_scan(sdmmc_host *host, boolean re_init);
int sdmmc_send_if_cond(sdmmc_host *host, uint32_t ocr);
int sdmmc_get_card_addr(sdmmc_host *host, uint32_t *rca);

#ifdef __cplusplus
}
#endif

#endif
