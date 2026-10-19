/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10be23
#define LOG_MODULE    AMOD_SDMMC

#include <aic_osa.h>
#include <sdmmc_common.h>
#include <sdmmc_core.h>
#include <sdhost_global.h>
#include <aic_errno.h>
//#include <aos_bitop.h>

/**---------------------------------------------------------------------------*/
/**                            macros define                                  */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                           global variables                                */
/**---------------------------------------------------------------------------*/
static const uint32_t tran_unit[] =
{
    10000, 100000, 1000000, 10000000,
    0,     0,      0,       0
};

static const uint8_t tran_value[] =
{
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static const uint32_t tacc_uint[] =
{
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

static const uint8_t tacc_value[] =
{
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

boolean b_using_ds_mode = AIC_FALSE;

/**---------------------------------------------------------------------------*/
/**                               functions                                   */
/**---------------------------------------------------------------------------*/
uint32_t SD_GET_BITS(uint32_t *resp, uint32_t start, uint32_t size)
{
    const int32_t __size = size;
    const uint32_t __mask = (__size < 32 ? 1 << __size : 0) - 1;
//    const int32_t __off = 3 - ((start) / 32);
    const int32_t __off = (start) / 32;
    const int32_t __shft = (start) & 31;
    uint32_t __res;

    __res = resp[__off] >> __shft;
    if (__size + __shft > 32)
        __res |= resp[__off+1] << ((32 - __shft) % 32);

    return __res & __mask;
}

/* Candence SDHC reponse register
Auto-CMD12 resp:    Response field R[39:8] - RESP3[31:0]
Auto-CMD23 resp:    Response field R[39:8] - RESP3[31:0]
No response:        RTS=00b
136-bit:            RTS=01b,
                    Response field R[127:8] - RESP3[23:0], RESP2[31:0], RESP1[31:0], RESP0[31:0], 
                    that means R[127:8] acturally refers to RESP[119:0]
48-bit:             RTS=10b,
                    Response field R[39:8] - RESP0[31:0]
48-bit with BUSY:   RTS=11b,
                    Response field R[39:8] - RESP0[31:0]
*/

static int sdmmc_parse_csd(sdmmc_card *card)
{
    sdmmc_csd *csd = &card->csd;
    uint32_t *resp = card->resp_csd;

    csd->csd_structure = SD_GET_BITS(resp, 118, 2);

    switch (csd->csd_structure)
    {
    case 0:
        csd->taac = SD_GET_BITS(resp, 104, 8);
        csd->nsac = SD_GET_BITS(resp, 96, 8);
        csd->tran_speed = SD_GET_BITS(resp, 88, 8);
        csd->card_cmd_class = SD_GET_BITS(resp, 76, 12);
        csd->rd_blk_len = SD_GET_BITS(resp, 72, 4);
        csd->rd_blk_part = SD_GET_BITS(resp, 71, 1);
        csd->wr_blk_misalign = SD_GET_BITS(resp, 70, 1);
        csd->rd_blk_misalign = SD_GET_BITS(resp, 69, 1);
        csd->dsr_imp = SD_GET_BITS(resp, 68, 1);
        csd->c_size = SD_GET_BITS(resp, 54, 12);
        csd->c_size_mult = SD_GET_BITS(resp, 39, 3);
        csd->r2w_factor = SD_GET_BITS(resp, 18, 3);
        csd->wr_blk_len = SD_GET_BITS(resp, 14, 4);
        csd->wr_blk_partial = SD_GET_BITS(resp, 13, 1);
//        csd->csd_crc = SD_GET_BITS(resp, 1, 7);

        card->card_blksize = 1 << csd->rd_blk_len;
        card->card_capacity = (csd->c_size + 1) << (csd->c_size_mult + 2);
        card->card_capacity *= card->card_blksize;
        card->card_capacity >>= 10; /* unit:KB */
        card->tacc_clks = csd->nsac * 100;
        card->tacc_ns = (tacc_uint[csd->taac&0x07] * tacc_value[(csd->taac&0x78)>>3] + 9) / 10;
        card->max_data_rate = tran_unit[csd->tran_speed&0x07] * tran_value[(csd->tran_speed&0x78)>>3];

        break;
    case 1:
        card->flags |= CARD_FLAG_SDHC;

        /*This field is fixed to 0Eh, which indicates 1 ms.
          The host should not use TAAC, NSAC, and R2W_FACTOR
          to calculate timeout and should uses fixed timeout
          values for read and write operations*/
        csd->taac = SD_GET_BITS(resp, 104, 8);
        csd->nsac = SD_GET_BITS(resp, 96, 8);
        csd->tran_speed = SD_GET_BITS(resp, 88, 8);
        csd->card_cmd_class = SD_GET_BITS(resp, 76, 12);
        csd->rd_blk_len = SD_GET_BITS(resp, 72, 4);
        csd->rd_blk_part = SD_GET_BITS(resp, 71, 1);
        csd->wr_blk_misalign = SD_GET_BITS(resp, 70, 1);
        csd->rd_blk_misalign = SD_GET_BITS(resp, 69, 1);
        csd->dsr_imp = SD_GET_BITS(resp, 68, 1);
        csd->c_size = SD_GET_BITS(resp, 40, 22);

        csd->r2w_factor = SD_GET_BITS(resp, 18, 3);
        csd->wr_blk_len = SD_GET_BITS(resp, 14, 4);
        csd->wr_blk_partial = SD_GET_BITS(resp, 13, 1);
//        csd->csd_crc = SD_GET_BITS(resp, 1, 7);

        card->card_blksize = 512;
        card->card_capacity = (csd->c_size + 1) * 512;  /* unit:KB */
        card->tacc_clks = 0;
        card->tacc_ns = 0;
        card->max_data_rate = tran_unit[csd->tran_speed&0x07] * tran_value[(csd->tran_speed&0x78)>>3];

        break;
    default:
        aicx_info(0x084f70, "d", "unrecognised CSD structure version %d!", csd->csd_structure);

        return AIC_EFAULT;
    }
    aicx_info(0x0dae38, "d", "SD card capacity: %d.", card->card_capacity);

    return 0;
}

int sdmmc_parse_scr(sdmmc_card *card)
{
    sd_scr *scr = &card->scr;
    uint32_t resp[4];

    resp[0] = card->resp_scr[1];
    resp[1] = card->resp_scr[0];
    scr->sd_version = SD_GET_BITS(resp, 56, 4);
    scr->sd_bus_widths = SD_GET_BITS(resp, 48, 4);

    return 0;
}

int sdmmc_switch(sdmmc_card *card)
{
    int32_t err = 0;
    sdmmc_host *host = card->host;
    sdmmc_req req;
    sdmmc_cmd cmd;
    sdmmc_data data;
    uint8_t *buf;
//    uint8_t i, mode = 1;  /* 0:DS Mode, 1:HS Mode */
//    uint32_t _status;

    buf = (uint8_t*)aos_malloc(64);
    if (!buf)
    {
        aicx_info(0x11080c, "", "alloc memory failed!");

        return AIC_ENOMEM;
    }

    if (card->card_type != CARD_TYPE_SD)
        goto ERR;
    if (card->scr.sd_version < SCR_SPEC_VER_1)
        goto ERR;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SD_SWITCH;
    cmd.arg = 0x00FFFFF1;
    cmd.flags = RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(sdmmc_data));

    sdmmc_set_data_timeout(&data, card);

    data.blksize = 64;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = (uint32_t *)buf;

    memset(&req, 0, sizeof(sdmmc_req));

    req.cmd = &cmd;
    req.data = &data;

    sdmmc_send_request(host, &req);

    if (cmd.err || data.err)
    {
        goto ERR;
    }

    if (buf[13] & 0x02) {
        /* support HighSpeed Mode */
        card->max_data_rate = 50000000;

        memset(&cmd, 0, sizeof(sdmmc_cmd));

        cmd.cmd_code = SD_SWITCH;
        cmd.arg = 0x80FFFFF1;
        cmd.flags = RESP_R1 | CMD_ADTC;

        memset(&data, 0, sizeof(sdmmc_data));

        sdmmc_set_data_timeout(&data, card);

        data.blksize = 64;
        data.blks = 1;
        data.flags = DATA_DIR_READ;
        data.buf = (uint32_t *)buf;

        memset(&req, 0, sizeof(sdmmc_req));

        req.cmd = &cmd;
        req.data = &data;

        sdmmc_send_request(host, &req);

        if (cmd.err || data.err)
        {
            goto ERR1;
        }

        if ((buf[16] & 0xF) != 1)
        {
            aicx_info(0x175c47, "", "switching card to high speed failed!");
            goto ERR;
        }

        card->flags |= CARD_FLAG_HIGHSPEED;
        aicx_info(0x1b5fa2, "", "switching card to high speed!");
    }

ERR:
    aos_free(buf);
    return 0;

ERR1:
    if (cmd.err)
        err = cmd.err;
    if (data.err)
        err = data.err;

    aos_free(buf);
    return err;

}

static int sdmmc_app_cmd(sdmmc_host *host, sdmmc_card *card)
{
    int err;
    sdmmc_cmd cmd = {0};

    cmd.cmd_code = APP_CMD;

    if (card)
    {
        cmd.arg = card->rca << 16;
        cmd.flags = RESP_R1 | CMD_AC;
    }
    else
    {
        cmd.arg = 0;
        cmd.flags = RESP_R1 | CMD_BCR;
    }

    err = sdmmc_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    /* Check that card supported application commands */
    if (!(cmd.resp[0] & R1_APP_CMD))
        return AIC_EACCES;

    return AIC_SUCCESS;
}


int sdmmc_send_app_cmd(sdmmc_host *host,
                            sdmmc_card *card,
                            sdmmc_cmd  *cmd,
                            int                   retry)
{
    sdmmc_req req;

    uint32_t i;
    int err;

    err = AIC_EFAULT;

    /*
     * We have to resend MMC_APP_CMD for each attempt so
     * we cannot use the retries field in mmc_command.
     */
    for (i = 0;i <= retry;i++)
    {
        memset(&req, 0, sizeof(sdmmc_req));

        err = sdmmc_app_cmd(host, card);
        if (err)
        {
            /* no point in retrying; no APP commands allowed */
            continue;
        }
    err = sdmmc_send_cmd(host, cmd, 0);
    if (err)
        return err;

    }

    return err;
}

int sdmmc_app_set_bus_width(sdmmc_card *card, int32_t width)
{
    uint32_t err;
    sdmmc_cmd cmd;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SD_APP_SET_BUS_WIDTH;
    cmd.flags = RESP_R1 | CMD_AC;

    switch (width)
    {
    case SDMMC_BUS_WIDTH_1:
        cmd.arg = SDMMC_BUS_WIDTH_1;
        break;
    case SDMMC_BUS_WIDTH_4:
        cmd.arg = SDMMC_BUS_WIDTH_4;
        break;
    default:
        return AIC_EINVAL;
    }

    err = sdmmc_send_app_cmd(card->host, card, &cmd, 0);
    if (err)
        return err;

    return AIC_SUCCESS;
}

int sdmmc_send_app_op_cond(sdmmc_host *host,
                                uint32_t           ocr,
                                uint32_t          *rocr)
{
    sdmmc_cmd cmd;
    uint32_t i;
    int err = AIC_SUCCESS;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SD_APP_OP_COND;
    cmd.arg = ocr;
    cmd.flags = RESP_R3 | CMD_BCR;

    for (i = 100; i; i--)
    {
        err = sdmmc_send_app_cmd(host, NULL, &cmd, 0);
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (cmd.resp[0] & CARD_BUSY)
            break;

        err = AIC_EIO;

        sdmmc_delay_ms(10); //delay 10ms
    }

    if (rocr)
        *rocr = cmd.resp[0];

    return err;
}

#if 0
/*
 * To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
 * before SD_APP_OP_COND. This command will harmlessly fail for
 * SD 1.0 cards.
 */
int sdmmc_send_if_cond(sdmmc_host *host, uint32_t ocr)
{
    sdmmc_cmd cmd;
    uint32_t err;
    uint8_t pattern;

    cmd.cmd_code = SD_SEND_IF_COND;
    cmd.arg = 0x1AA;//((ocr & 0xFF8000) != 0) << 8 | 0xAA;
    cmd.flags = RESP_R7 | CMD_BCR;

    err = sdmmc_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    pattern = cmd.resp[0] & 0xFF;

    if (pattern != 0xAA)
        return AIC_EIO;

    return AIC_SUCCESS;
}

int sdmmc_get_card_addr(sdmmc_host *host, uint32_t *rca)
{
    uint32_t err;
    sdmmc_cmd cmd;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SD_SEND_RELATIVE_ADDR;
    cmd.arg = 0;
    cmd.flags = RESP_R6 | CMD_BCR;

    err = sdmmc_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    *rca = cmd.resp[0] >> 16;

    return AIC_SUCCESS;
}
#endif
#define be32_to_cpu(x) ((uint32_t)(              \
    (((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |        \
    (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |        \
    (((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |        \
    (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

int sdmmc_get_scr(sdmmc_card *card, uint32_t *scr)
{
    int32_t err;
    sdmmc_req req;
    sdmmc_cmd cmd;
    sdmmc_data data;

    err = sdmmc_app_cmd(card->host, card);
    if (err)
        return err;

    memset(&req, 0, sizeof(sdmmc_req));
    memset(&cmd, 0, sizeof(sdmmc_cmd));
    memset(&data, 0, sizeof(sdmmc_data));

    req.cmd = &cmd;
    req.data = &data;

    cmd.cmd_code = SD_APP_SEND_SCR;
    cmd.arg = 0;
    cmd.flags = RESP_R1 | CMD_ADTC;

    data.blksize = 8;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = scr;

    sdmmc_set_data_timeout(&data, card);

    sdmmc_send_request(card->host, &req);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    scr[0] = be32_to_cpu(scr[0]);
    scr[1] = be32_to_cpu(scr[1]);

    return 0;
}


static int sdmmc_sd_init_card(sdmmc_host *host, uint32_t ocr, boolean re_init)
{
    sdmmc_card *card;
    int32_t err;
    uint32_t resp[4];

    sdmmc_go_idle(host);

    /*
     * If SD_SEND_IF_COND indicates an SD 2.0
     * compliant card and we should set bit 30
     * of the ocr to indicate that we can handle
     * block-addressed SDHC cards.
     */
    err = sdmmc_send_if_cond(host, ocr);
    if (!err)
        ocr |= 1 << 30;

    err = sdmmc_send_app_op_cond(host, ocr, NULL);
    if (err) {
        /* try with HCS=0 to be compatible with some bad TCards */
        ocr &= ~(1<<30);
        err = sdmmc_send_app_op_cond(host, ocr, NULL);
        if (err)
            goto err;
    }

    err = sdmmc_all_get_cid(host, resp);
    if (err)
        goto err;

    if (re_init)
        card = host->card;
    else
        card = aos_malloc(sizeof(sdmmc_card));

    if (!card)
    {
        aicx_info(0x0b2e13, "", "malloc card failed!");
        err = AIC_ENOMEM;
        goto err;
    }
    memset(card, 0, sizeof(sdmmc_card));

    host->card = card;
    card->card_type = CARD_TYPE_SD;
    card->host = host;
    memcpy(card->resp_cid, resp, sizeof(card->resp_cid));

    /*
     * For native busses:  get card RCA and quit open drain mode.
     */
    err = sdmmc_get_card_addr(host, &card->rca);
    if (err)
        goto err1;

    //sdmmc_set_bus_mode(host, sdmmc_BUSMODE_PUSHPULL);

    err = sdmmc_get_csd(card, card->resp_csd);
    if (err)
        goto err1;

    err = sdmmc_parse_csd(card);
    if (err)
        goto err1;

    err = sdmmc_select_card(card);
    if (err)
        goto err1;

    err = sdmmc_get_scr(card, card->resp_scr);
    if (err)
        goto err1;

    sdmmc_parse_scr(card);

    if (!b_using_ds_mode) {
        /*
         * change SD card to high-speed, only SD2.0 spec
         */
        err = sdmmc_switch(card);
        if (err)
            goto err1;
    }

    /* set bus speed */
    if (card->flags & CARD_FLAG_HIGHSPEED)
        sdmmc_set_dfi(host, SDCARD_SPEED_HS, 0);
    else
        sdmmc_set_dfi(host, SDCARD_SPEED_DS, 0);

    sdmmc_set_clock(host, card->max_data_rate);

    if (!b_using_ds_mode) {
        /*switch bus width*/
        if ((host->flags & SDMMC_BUSWIDTH_4) &&
            (card->scr.sd_bus_widths & SD_SCR_BUS_WIDTH_4))
        {
            err = sdmmc_app_set_bus_width(card, SDMMC_BUS_WIDTH_4);
            if (err)
                goto err1;

            sdmmc_set_bus_width(host, SDMMC_BUS_WIDTH_4);
        }
    }

    return 0;

err1:
    if (!re_init) {
        aos_free(card);
        host->card = AIC_NULL;
    }

err:
    return err;
}

/*
 * Starting point for SD card init.
 */
int init_sd(sdmmc_host *host, uint32_t ocr, boolean re_init)
{
    int32_t err = AIC_SUCCESS;
    uint32_t  current_ocr;

    if (ocr & VDD_165_195)
    {
#if 0
        aicx_info(0x0d8c9b, "", " SD card claims to support the "
               "incompletely defined 'low voltage range'. This "
               "will be ignored.");
#endif
        ocr &= ~VDD_165_195;
    }

    current_ocr = sdmmc_select_voltage(host, ocr);

    /*
     * Can we support the voltage(s) of the card(s)?
     */
    if (!current_ocr)
    {
        return AIC_EACCES;
    }

    /*
     * Detect and init the card.
     */
    err = sdmmc_sd_init_card(host, current_ocr, re_init);
    return err;
}

