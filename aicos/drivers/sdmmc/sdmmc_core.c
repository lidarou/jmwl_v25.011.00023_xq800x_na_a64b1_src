/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/* aic header */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1750bb
#define LOG_MODULE    AMOD_SDMMC

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_bitmap.h>
#include <irq.h>
#include <reg_base.h>
#include <mcu_glb.h>
#include <aic_assert.h>
#include "sdmmc_core.h"
#include "sdhost_global.h"
#include "sdmmc_common.h"
#include "dfi.h"
#ifdef CONFIG_SDMMC_SD
#include "sd.h"
#endif
#ifdef CONFIG_SDMMC_SDIO
#include "sdio.h"
#endif

/**---------------------------------------------------------------------------*/
/**                            macros define                                  */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                           global variables                                */
/**---------------------------------------------------------------------------*/
sdmmc_host_rpm g_rpm;
u32 g_wait_option = AOS_WAIT_FOREVER;
static aos_thread_id sdmmc_detect_thread;
sdmmc_host *g_host = AIC_NULL;
extern boolean b_using_ds_mode;

/**---------------------------------------------------------------------------*/
/**                               functions                                   */
/**---------------------------------------------------------------------------*/
int sdmmc_add(sdmmc_card *card);
int sdmmc_remove(sdmmc_card *card);
static void sdmmc_power_off(sdmmc_host *host);

void sdmmc_host_lock(sdmmc_host *host)
{
    if (host->host_lock)
        aos_get_mutex(host->host_lock, AOS_WAIT_FOREVER);
}

void sdmmc_host_unlock(sdmmc_host *host)
{
    if (host->host_lock)
        aos_put_mutex(host->host_lock);
}

void sdmmc_req_complete(sdmmc_host *host)
{
    if (host->sdmmc_mutex)
        aos_put_mutex(host->sdmmc_mutex);
}

void sdmmc_send_request(sdmmc_host *host, sdmmc_req *req)
{
    int err = 0x0;

    if (host->sdmmc_mutex)
        aos_get_mutex(host->sdmmc_mutex, AOS_WAIT_FOREVER);

#ifdef CONFIG_SDMMC_SD
    if (g_rpm.lowpower) {
        aicx_info(0x0f184d, "", "sdmmc_send_request: sdmmc status:lowpower, need re-init !\n");
        err = sdmmc_core_scan(host, AIC_TRUE);
        if (err) {
            aicx_info(0x0c9c14, "", "sdmmc_send_request: re-init fail !\n");
            req->cmd->err = err;
            return;
        }

        sdhost_send_rpm();
        g_rpm.lowpower = AIC_FALSE;
    }
    g_wait_option = SDMMC_RPM_TIME;
    g_rpm.rpm_checkcnt = SDMMC_RPM_CNT;
#endif

    do {
        req->cmd->retries--;
        req->cmd->err = 0;
        req->cmd->mrq = req;
        if (req->data)
        {
            req->cmd->data = req->data;
            req->data->err = 0;
            req->data->mrq = req;
            if (req->stop)
            {
                req->data->stop = req->stop;
                req->stop->err = 0;
                req->stop->mrq = req;
            }
        }
        host->ops->request(host, req);

    } while(req->cmd->err && (req->cmd->retries > 0));

#if 1
    /* power off and re-init to try */
    if (req->data && (req->data->err||req->cmd->err)) {
        aicx_info(0x10f0cc, "dd", "sdmmc_send_request failed and retry! failed cmd: %d, err: %d!\n", req->cmd->cmd_code, req->cmd->err);
        if (req->data)
            aicx_info(0x192f7a, "d", "req->data->err: %d!\n", req->data->err);

        sdmmc_power_off(host);
        /* using ds_modem and 1-bit to retry */
        b_using_ds_mode = AIC_TRUE;

        if (host->sdmmc_mutex)
            aos_get_mutex(host->sdmmc_mutex, AOS_WAIT_FOREVER);

        err = sdmmc_core_scan(host, AIC_TRUE);
        if (err) {
            aicx_info(0x0c9c14, "", "sdmmc_send_request: re-init fail !\n");
            req->cmd->err = err;
            return;
        }

        req->cmd->retries = 1;

        do {
            req->cmd->retries--;
            req->cmd->err = 0;
            req->cmd->mrq = req;
            if (req->data)
            {
                req->cmd->data = req->data;
                req->data->err = 0;
                req->data->mrq = req;
                if (req->stop)
                {
                    req->data->stop = req->stop;
                    req->stop->err = 0;
                    req->stop->mrq = req;
                }
            }
            host->ops->request(host, req);

        } while(req->cmd->err && (req->cmd->retries > 0));
    }
#endif

}

int32_t sdmmc_send_cmd(sdmmc_host *host, sdmmc_cmd  *cmd, int       retries)
{
#if 0
    sdmmc_req sdmmcreq, *req;

    req = &sdmmcreq;
    memset(&sdmmcreq, 0, sizeof(sdmmc_req));
    memset(cmd->resp, 0, sizeof(cmd->resp));
    cmd->retries = retries;

    req->cmd = cmd;
    cmd->data = NULL;

    do {
        req->cmd->retries--;
        req->cmd->err = 0;
        req->cmd->mrq = req;
        if (req->data)
        {
            req->cmd->data = req->data;
            req->data->err = 0;
            req->data->mrq = req;
            if (req->stop)
            {
                req->data->stop = req->stop;
                req->stop->err = 0;
                req->stop->mrq = req;
            }
        }
        host->ops->request(host, req);
    } while(req->cmd->err && (req->cmd->retries > 0));

    return cmd->err;
#else
    sdmmc_req req;

    memset(&req, 0, sizeof(sdmmc_req));
    memset(cmd->resp, 0, sizeof(cmd->resp));
    cmd->retries = retries;

    req.cmd = cmd;
    cmd->data = AIC_NULL;

    sdmmc_send_request(host, &req);

    return cmd->err;
#endif

}

int32_t sdmmc_go_idle(sdmmc_host *host)
{
    int32_t err;
    sdmmc_cmd cmd;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = GO_IDLE_STATE;
    cmd.arg = 0;
    cmd.flags = RESP_NONE | CMD_BC;

    err = sdmmc_send_cmd(host, &cmd, 0);

    sdmmc_delay_ms(1);

    return err;
}

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

int32_t sdmmc_all_get_cid(sdmmc_host *host, uint32_t *cid)
{
    int32_t err;
    sdmmc_cmd cmd;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = ALL_SEND_CID;
    cmd.arg = 0;
    cmd.flags = RESP_R2 | CMD_BCR;

    err = sdmmc_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    memcpy(cid, cmd.resp, sizeof(uint32_t) * 4);

    return 0;
}

int32_t sdmmc_get_csd(sdmmc_card *card, uint32_t *csd)
{
    int32_t err;
    sdmmc_cmd cmd;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SEND_CSD;
    cmd.arg = card->rca << 16;
    cmd.flags = RESP_R2 | CMD_AC;
    err = sdmmc_send_cmd(card->host, &cmd, 0);
    if (err)
        return err;

    memcpy(csd, cmd.resp, sizeof(uint32_t) * 4);

    return 0;

}

static int32_t _sdmmc_select_card(sdmmc_host *host, sdmmc_card *card)
{
    int32_t err;
    sdmmc_cmd cmd;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SELECT_CARD;

    if (card)
    {
        cmd.arg = card->rca << 16;
        cmd.flags = RESP_R1 | CMD_AC;
    }
    else
    {
        cmd.arg = 0;
        cmd.flags = RESP_NONE | CMD_AC;;
    }

    err = sdmmc_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    return 0;
}

int32_t sdmmc_select_card(sdmmc_card *card)
{
    return _sdmmc_select_card(card->host, card);
}

int32_t sdmmc_deselect_cards(sdmmc_card *card)
{
    return _sdmmc_select_card(card->host, NULL);
}

int32_t sdmmc_spi_use_crc(sdmmc_host *host, int32_t use_crc)
{
    sdmmc_cmd cmd;
    int32_t err;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SPI_CRC_ON_OFF;
    cmd.flags = RESP_SPI_R1;
    cmd.arg = use_crc;

    err = sdmmc_send_cmd(host, &cmd, 0);
    if (!err)
        host->spi_use_crc = use_crc;

    return err;
}

/*
 * Control chip select pin on a host.
 */
void sdmmc_set_chip_select(sdmmc_host *host, int32_t mode)
{
}

void sdmmc_set_dfi(sdmmc_host *host, uint32_t sdcard_speed, uint32_t uhsi_mode)
{
    uint32_t tmp;
    sdmmc_card *card = (sdmmc_card *)host->card;

    switch (sdcard_speed) {
    case SDCARD_SPEED_DS:
        card->max_data_rate = 25000000;
        dfi_ds();
        tmp = sdhost_read_phy_reg(0x2000);
        tmp |= (0x1<<16);
        sdhost_set_phy_reg(0x2000, tmp);
        break;
    case SDCARD_SPEED_HS:
        card->max_data_rate = 50000000;
        dfi_hs();
        tmp = sdhost_read_phy_reg(0x2000);
        tmp |= (0x1<<16);
        sdhost_set_phy_reg(0x2000, tmp);
        break;
    case SDCARD_SPEED_UHS_I:
        switch (uhsi_mode) {
        case UHSI_SDR12:
            card->max_data_rate = 25000000;
            tmp = sdhost_read_phy_reg(0x2000);
            tmp &= ~(0x1<<16);
            sdhost_set_phy_reg(0x2000, tmp);
            break;
        case UHSI_SDR25:
            card->max_data_rate = 50000000;
            tmp = sdhost_read_phy_reg(0x2000);
            tmp &= ~(0x1<<16);
            sdhost_set_phy_reg(0x2000, tmp);
            break;
        case UHSI_SDR50:
            card->max_data_rate = 100000000;
            dfi_sdr50();
            tmp = sdhost_read_phy_reg(0x2000);
            tmp &= ~(0x1<<16);
            sdhost_set_phy_reg(0x2000, tmp);
            break;
        case UHSI_SDR104:
            card->max_data_rate = 100000000;
            //dfi_sdr104();
            tmp = sdhost_read_phy_reg(0x2000);
            tmp &= ~(0x1<<16);
            sdhost_set_phy_reg(0x2000, tmp);
            break;
        case UHSI_DDR50:
            card->max_data_rate = 50000000;
            dfi_ddr50();
            tmp = sdhost_read_phy_reg(0x2000);
            tmp &= ~(0x1<<16);
            sdhost_set_phy_reg(0x2000, tmp);
            break;
        default:
            card->max_data_rate = 50000000;
            break;
        }
        break;
    default:
        card->max_data_rate = 50000000;
        break;
    }
}

/*
 * Sets the host clock to the highest possible frequency that
 * is below "hz".
 */
void sdmmc_set_clock(sdmmc_host *host, uint32_t clk)
{
    if (clk < host->freq_min)
    {
        aicx_info(0x110436, "", "clock too low!");
    }

    sdhost_set_clk(host, clk);
}

/*
 * Change the bus mode (open drain/push-pull) of a host.
 */
void sdmmc_set_bus_mode(sdmmc_host *host, uint32_t mode)
{
}

/*
 * Change data bus width of a host.
 */
void sdmmc_set_bus_width(sdmmc_host *host, uint32_t width)
{
    sdhost_set_width(width);
}

void sdmmc_set_data_timeout(sdmmc_data       *data,
                            const sdmmc_card *card)
{

}

/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
uint32_t sdmmc_select_voltage(sdmmc_host *host, uint32_t ocr)
{
    return ocr;
}

static void sdmmc_power_up(sdmmc_host *host)
{
    sdhost_set_power(host, VOLTAGE_3_3, TRUE);
    /*
     * This delay must be at least 74 clock sizes, or 1 ms, or the
     * time required to reach a stable voltage.
     */
    sdmmc_delay_ms(10);

    sdhost_set_clk(host, 398000);
    sdmmc_delay_ms(5);
}

static void sdmmc_power_off(sdmmc_host *host)
{
    sdhost_disable_clk();
    sdhost_enable(AIC_FALSE);
    sdhost_set_power(host, VOLTAGE_3_3, FALSE);
    sdmmc_delay_ms(10);

}

int sdmmc_core_scan(sdmmc_host *host, boolean re_init)
{
    uint32_t ocr = VDD_33_34 | VDD_32_33 | VDD_31_32 | VDD_30_31 | VDD_165_195;
    int32_t err;

#ifdef CONFIG_SDMMC_SDIO
    /* detect SDIO card */
    sdhost_clk_src_sel();
    sdhost_enable(AIC_TRUE);
    sdhost_config(host);
    sdmmc_power_up(host);

    err = sdio_io_send_op_cond(host, 0, &ocr);
    if (!err) {
        if (init_sdio(host, ocr)) {
            sdmmc_power_off(host);
            aicx_info(0x020eef, "", "SDIO Card Initialize fail!\r\n");
            return -1;
        }
    }
#endif

#ifdef CONFIG_SDMMC_SD
    u8 retry_cnt = 0;

    if (!re_init)
        b_using_ds_mode = AIC_FALSE;

    do {
        if (retry_cnt == 1)
            b_using_ds_mode = AIC_TRUE;

        g_wait_option = SDMMC_RPM_TIME;
        g_rpm.rpm_checkcnt = SDMMC_RPM_CNT;
        g_rpm.lowpower = AIC_FALSE;

        sdhost_clk_src_sel();
        sdhost_enable(AIC_TRUE);
        sdhost_config(host);
        sdmmc_power_up(host);

        sdmmc_go_idle(host);
        sdmmc_send_if_cond(host, host->valid_ocr);

        /* detect SD memory card */
        err = sdmmc_send_app_op_cond(host, 0, &ocr);
        if (!err) {
            err = init_sd(host, ocr, re_init);
            if(!err) 
                break;    /* initialize success, break */
        }

        if (!g_rpm.lowpower) {
            sdmmc_power_off(host);
            g_rpm.lowpower = AIC_TRUE;
        }
        aicx_info(0x05214a, "dd", "SD Card Initialize fail %d times, errcode: %d!\r\n", retry_cnt, err);
        retry_cnt++;
    } while(retry_cnt <= 1);
#endif

    return err;
}

void sdmmc_detect(void *param)
{
    u32 ret;
    int err = 0x0;
    sdmmc_host *host = AIC_NULL;

    sdhost_init();

    while (1)
    {
        ret = sdhost_detect_card(g_wait_option);

        /* detect card inserted */
        if (ret == SDMMC_DETECT_INSERT) {
            /* card inserted */
            aicx_info(0x1c1906, "", "sdmmc_detect: card inserted !\n");
            host = sdmmc_alloc_host();
            AIC_ASSERT(host);
            err = sdmmc_core_scan(host, AIC_FALSE);
#ifndef CONFIG_SDMMC_SDIO
            if (!err)
                sdmmc_add(host->card);
#endif
        }

        /* detect card removed */
        else if (ret == SDMMC_DETECT_REMOVE) {
            /* card removed */
            aicx_info(0x097e16, "", "sdmmc_detect: card removed !\n");
            g_wait_option = AOS_WAIT_FOREVER;
            if (host) {
#ifndef CONFIG_SDMMC_SDIO
                sdmmc_remove(host->card);
#endif
                if (!g_rpm.lowpower) {
                    sdmmc_power_off(host);
                    g_rpm.lowpower = AIC_TRUE;
                }
                sdhost_disconfig(host);
                sdmmc_free_host(host);
                host = AIC_NULL;
            }
        }

        /* detect realtime power manage event, only for SD Card */
#ifdef CONFIG_SDMMC_SD
        else if (ret == SDMMC_DETECT_RPM) {
            if (g_rpm.rpm_checkcnt) {
                aicx_info(0x00b413, "u", "sdmmc_detect: RPM check cnt %u !\n", g_rpm.rpm_checkcnt);
                g_wait_option = SDMMC_RPM_TIME;
                /* get mutex before modify rpm_checkcnt */
                if (host) {
                    sdmmc_host_lock(host);
                    g_rpm.rpm_checkcnt--;
                    if (g_rpm.rpm_checkcnt == 0) {
                        g_wait_option = AOS_WAIT_FOREVER;
                        // power off
                        if (!g_rpm.lowpower) {
                            sdmmc_power_off(host);
                            g_rpm.lowpower = AIC_TRUE;
                        }
                        aicx_info(0x107119, "", "sdmmc_detect: RPM power off!\n");
                    }
                    sdmmc_host_unlock(host);
                }
            }
        }
#endif
        /* other cases */
        else
            AOS_ASSERT(0);
    }
}

sdmmc_host *sdmmc_alloc_host(void)
{
    sdmmc_host *host;

    host = aos_malloc(sizeof(sdmmc_host));
    if (!host)
    {
        aicx_info(0x16addb, "", "alloc host failed");
        return NULL;
    }

    memset(host, 0, sizeof(sdmmc_host));

    host->freq_min = 300000;
    host->freq_max = 50000000;
    host->valid_ocr = VDD_32_33 | VDD_33_34;
    host->flags = SDMMC_MUTBLKWRITE | SDMMC_SUP_HIGHSPEED | SDMMC_SUP_SDIO_IRQ | SDMMC_BUSWIDTH_4;
    host->max_seg_size = 2048;
    host->max_dma_segs = 10;
    host->max_blk_size = 512;
    host->max_blk_count = 4096;

    host->max_seg_size = 65535;
    host->max_dma_segs = 1;
    host->max_blk_size = 512;
    host->max_blk_count = 4096;

    host->sdmmc_event = aos_create_event("sdmmc");
    AIC_ASSERT (host->sdmmc_event);

    host->host_lock = aos_create_mutex("sdmmc_host_lock");
    AIC_ASSERT (host->host_lock);

    host->sdmmc_mutex = aos_create_mutex("sdmmc_mutex");
    AIC_ASSERT (host->sdmmc_mutex);

    g_host = host;
    return host;
}

void sdmmc_free_host(sdmmc_host *host)
{
    if (host) {
        if (host->host_lock) {
            aos_delete_mutex(host->host_lock);
            host->host_lock = AIC_NULL;
        }

        if (host->sdmmc_event) {
            aos_delete_event(host->sdmmc_event);
            host->sdmmc_event = AIC_NULL;
        }

        if (host->sdmmc_mutex) {
            aos_delete_mutex(host->sdmmc_mutex);
            host->sdmmc_mutex = AIC_NULL;
        }

        if (host->card) {
            aos_free(host->card);
            host->card = AIC_NULL;
        }

        host->ops = AIC_NULL;
        aos_free(host);
        g_host = NULL;
    }
}

int sdmmc_core_init(void)
{
    sdmmc_detect_thread = aos_create_thread_simply("sdmmc_detect", sdmmc_detect, AOS_NULL, 4096, 17);

    return 0;
}

AIC_NORMAL_DRV_L1_INIT_EXPORT(sdmmc_core_init);

