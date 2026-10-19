/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05ba21
#define LOG_MODULE    AMOD_SDMMC

#include <aic_osa.h>
#include <sdmmc_common.h>
#include <sdmmc_core.h>
#include <sdhost_global.h>
#include <sdio.h>
#include <aic_errno.h>
#include <errno.h>
#include <aic_list.h>
#include <user_tracex_api.h>

/**---------------------------------------------------------------------------*/
/**                            macros define                                  */
/**---------------------------------------------------------------------------*/
#ifndef SDIO_STACK_SIZE
#define SDIO_STACK_SIZE 2048
#endif
#ifndef SDIO_THREAD_PRIORITY
#define SDIO_THREAD_PRIORITY 12
#endif

//#define MIN(a, b) (a < b ? a : b)

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/
typedef struct sdio_card_st
{
    sdmmc_card *card;
    struct list_head  list;
} sdio_card;

typedef struct sdio_driver_st
{
    _sdio_driver *drv;
    struct list_head  list;
} sdio_driver;

/**---------------------------------------------------------------------------*/
/**                           global variables                                */
/**---------------------------------------------------------------------------*/
static struct list_head sdio_cards = LIST_HEAD_INIT(sdio_cards);
static struct list_head sdio_drivers = LIST_HEAD_INIT(sdio_drivers);

static const uint8_t speed_value[16] =
{
    0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80
};

static const uint32_t speed_unit[8] =
{
    10000, 100000, 1000000, 10000000, 0, 0, 0, 0
};

/**---------------------------------------------------------------------------*/
/**                               functions                                   */
/**---------------------------------------------------------------------------*/
extern void ssv_wifi_init(void);

int32_t sdio_io_send_op_cond(sdmmc_host *host,
                                uint32_t           ocr,
                                uint32_t          *cmd5_resp)
{
    sdmmc_cmd cmd;
    int32_t i, err = 0;

    AOS_ASSERT(host != NULL);

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SD_IO_SEND_OP_COND;
    cmd.arg = ocr;
    cmd.flags = RESP_SPI_R4 | RESP_R4 | CMD_BCR;

    for (i = 100; i; i--)
    {
        err = sdmmc_send_cmd(host, &cmd, 0);
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (controller_is_spi(host))
        {
            /*
             * Both R1_SPI_IDLE and MMC_CARD_BUSY indicate
             * an initialized card under SPI, but some cards
             * (Marvell's) only behave when looking at this
             * one.
             */
            if (cmd.resp[1] & CARD_BUSY)
                break;
        }
        else
        {
            if (cmd.resp[0] & CARD_BUSY)
                break;
        }

        err = -ETIME;

        sdmmc_delay_ms(10);
    }

    if (cmd5_resp)
        *cmd5_resp = cmd.resp[controller_is_spi(host) ? 1 : 0];

    return err;
}

int32_t sdio_io_rw_direct(sdmmc_card *card,
                             int32_t            rw,
                             uint32_t           fn,
                             uint32_t           reg_addr,
                             uint8_t           *pdata,
                             uint8_t            raw)
{
    sdmmc_cmd cmd;
    int32_t err;

    //USER_TRACE_IN_LINE_INSERT(5000, 0xCC, 0x01);

#if TRACE_SD_CMD_RSP_DETAILS
    aicx_info(0x03bbf5, "ddX", "sdio_io_rw_direct enter, rw: %d, fn: %d, reg_addr: 0x%X !\r\n", rw, fn, reg_addr);
#endif

    AOS_ASSERT(card != NULL);
    AOS_ASSERT(fn <= SDIO_MAX_FUNCTIONS);
    AOS_ASSERT(pdata != NULL);

    if (reg_addr & ~SDIO_ARG_CMD53_REG_MASK)
        return -EFAULT;

    memset(&cmd, 0, sizeof(sdmmc_cmd));

    cmd.cmd_code = SD_IO_RW_DIRECT;
    cmd.arg = rw ? SDIO_ARG_CMD52_WRITE : SDIO_ARG_CMD52_READ;
    cmd.arg |= fn << SDIO_ARG_CMD52_FUNC_SHIFT;
    cmd.arg |= raw ? SDIO_ARG_CMD52_RAW_FLAG : 0x00000000;
    cmd.arg |= reg_addr << SDIO_ARG_CMD52_REG_SHIFT;
    cmd.arg |= *pdata;
    cmd.flags = RESP_SPI_R5 | RESP_R5 | CMD_AC;

    err = sdmmc_send_cmd(card->host, &cmd, 0);
    if (err) {
        aicx_info(0x00d2e1, "d", "sdio_io_rw_direct: send cmd error: %d\r\n", err);
        return err;
    }
    if (!controller_is_spi(card->host))
    {
        if (cmd.resp[0] & R5_ERROR)
            err = -EIO;
        if (cmd.resp[0] & R5_FUNCTION_NUMBER)
            err = -EFAULT;
        if (cmd.resp[0] & R5_OUT_OF_RANGE)
            err = -EFAULT;
        if (err) {
            aicx_info(0x0af810, "Xd", "sdio_io_rw_direct fail, response: 0x%X, err: %d\r\n", cmd.resp[0], err);
            return err;
        }
    }

    if (!rw || raw)
    {
        if (controller_is_spi(card->host))
            *pdata = (cmd.resp[0] >> 8) & 0xFF;
        else
            *pdata = cmd.resp[0] & 0xFF;
    }

    //USER_TRACE_IN_LINE_INSERT(5000, 0xCC, 0x02);

#if TRACE_SD_CMD_RSP_DETAILS
    aicx_info(0x0008ed, "", "sdio_io_rw_direct success!\r\n");
#endif

    return 0;
}

int32_t sdio_io_rw_extended(sdmmc_card *card,
                               int32_t            rw,
                               uint32_t           fn,
                               uint32_t           addr,
                               int32_t            op_code,
                               uint8_t           *buf,
                               uint32_t           blocks,
                               uint32_t           blksize)
{
    sdmmc_req req;
    sdmmc_cmd cmd;
    sdmmc_data data;
    int32_t err = 0;

    AOS_ASSERT(card != NULL);
    AOS_ASSERT(fn <= SDIO_MAX_FUNCTIONS);
    AOS_ASSERT(blocks != 1 || blksize <= 512);
    AOS_ASSERT(blocks != 0);
    AOS_ASSERT(blksize != 0);

    if (addr & ~SDIO_ARG_CMD53_REG_MASK)
        return -EFAULT;

    memset(&req, 0, sizeof(sdmmc_req));
    memset(&cmd, 0, sizeof(sdmmc_cmd));
    memset(&data, 0, sizeof(sdmmc_data));

    req.cmd = &cmd;
    req.data = &data;

    cmd.cmd_code = SD_IO_RW_EXTENDED;
    cmd.arg = rw ? SDIO_ARG_CMD53_WRITE : SDIO_ARG_CMD53_READ;
    cmd.arg |= fn << SDIO_ARG_CMD53_FUNC_SHIFT;
    cmd.arg |= op_code ? SDIO_ARG_CMD53_INCREMENT : 0x00000000;
    cmd.arg |= addr << SDIO_ARG_CMD53_REG_SHIFT;
    if (blocks == 1 && blksize <= 512)
        cmd.arg |= (blksize == 512) ? 0 : blksize;      /* byte mode */
    else
        cmd.arg |= SDIO_ARG_CMD53_BLOCK_MODE | blocks;  /* block mode */
    cmd.flags = RESP_SPI_R5 | RESP_R5 | CMD_ADTC;

    data.blksize = blksize;
    data.blks = blocks;
    data.flags = rw ? DATA_DIR_WRITE : DATA_DIR_READ;
    data.buf = (uint32_t *)buf;

    sdmmc_set_data_timeout(&data, card);

    sdmmc_send_request(card->host, &req);

    if (cmd.err) {
        aicx_info(0x0c04d0, "d", "sdio_io_rw_extended fail, cmd.err: %d !\r\n", cmd.err);
        return cmd.err;
    }
    if (data.err) {
        aicx_info(0x10cf68, "d", "sdio_io_rw_extended fail, data.err: %d !\r\n", data.err);
        return data.err;
    }
    if (!controller_is_spi(card->host))
    {
        if (cmd.resp[0] & R5_ERROR)
            err = -EIO;
        if (cmd.resp[0] & R5_FUNCTION_NUMBER)
            err = -EFAULT;
        if (cmd.resp[0] & R5_OUT_OF_RANGE)
            err = -EFAULT;
        if (err) {
            aicx_info(0x0d16d3, "Xd", "sdio_io_rw_extended fail, response: 0x%X, err: %d\r\n", cmd.resp[0], err);
        return err;
        }
    }

    return 0;
}

uint32_t sdio_max_block_size(sdio_function *func)
{
    sdmmc_host *host = func->card->host;
    uint32_t size = MIN(host->max_seg_size, host->max_blk_size);
    size = MIN(size, func->max_blk_size);

    return MIN(size, 512u); /* maximum size for byte mode */
}

int32_t sdio_io_rw_extended_block(sdio_function *func,
                                            int32_t               rw,
                                            uint32_t              addr,
                                            int32_t               op_code,
                                            uint8_t              *buf,
                                            uint32_t              len)
{
    int32_t  ret;
    uint32_t left_size;
    uint32_t max_blks, blks;
    sdmmc_host *host = func->card->host;

    //USER_TRACE_IN_LINE_INSERT(5000, 0xBB, 0x01);

#if TRACE_SD_CMD_RSP_DETAILS
    aicx_info(0x1b3575, "XXddd", "sdio_io_rw_extended_block enter, addr: 0x%X, buf: 0x%X, len: %d, rw: %d, op_code: %d !\r\n", addr, buf, len, rw, op_code);
#endif

    AOS_ASSERT(func != NULL);
    AOS_ASSERT(func->card != NULL);

    left_size = len;

    /* Do the bulk of the transfer using block mode (if supported). */
    if (func->card->cccr.multi_block && (len > sdio_max_block_size(func)))
    {
        max_blks = MIN(host->max_blk_count, host->max_seg_size / func->cur_blk_size);
        max_blks = MIN(max_blks, 511u);

        while (left_size > func->cur_blk_size)
        {
            blks = left_size / func->cur_blk_size;
            if (blks > max_blks)
                blks = max_blks;
            len = blks * func->cur_blk_size;

            ret = sdio_io_rw_extended(func->card, rw, func->num,
                  addr, op_code, buf, blks, func->cur_blk_size);
            if (ret)
                return ret;

            left_size -= len;
            buf += len;
            if (op_code)
                addr += len;
        }
    }

    while (left_size > 0)
    {
        len = MIN(left_size, sdio_max_block_size(func));

        ret = sdio_io_rw_extended(func->card, rw, func->num,
                  addr, op_code, buf, 1, len);
        if (ret)
            return ret;

        left_size -= len;
        buf += len;
        if (op_code)
            addr += len;
    }

    //USER_TRACE_IN_LINE_INSERT(5000, 0xBB, 0x02);

#if TRACE_SD_CMD_RSP_DETAILS
    aicx_info(0x1666e5, "", "sdio_io_rw_extended_block success!\r\n");
#endif

    return 0;
}

uint8_t sdio_io_readb(sdio_function *func,
                         uint32_t              reg,
                         int32_t              *err)
{
    uint8_t data = 0;
    int32_t ret;

    ret = sdio_io_rw_direct(func->card, 0, func->num, reg, &data, 0);

    if (err)
    {
        *err = ret;
    }

#if TRACE_SD_CMD_RSP_DETAILS
    aicx_info(0x03f457, "Xd", "sdio_io_readb, data = 0x%X, errno = %d\r\n", data, ret);
#endif

    return data;
}

int32_t sdio_io_writeb(sdio_function *func,
                          uint32_t              reg,
                          uint8_t               data)
{
    int32_t ret;
#if TRACE_SD_CMD_RSP_DETAILS
    uint8_t data_w = data;
#endif

    ret = sdio_io_rw_direct(func->card, 1, func->num, reg, &data, 1);

#if TRACE_SD_CMD_RSP_DETAILS
    aicx_info(0x083cd2, "XXd", "sdio_io_writeb, data_w = 0x%X, data_r = 0x%X, errno = %d\r\n", data_w, data, ret);
#endif

    return ret;
}

uint16_t sdio_io_readw(sdio_function *func,
                          uint32_t              addr,
                          int32_t              *err)
{
    int32_t ret;
    uint32_t dmabuf;

    if (err)
        *err = 0;

    ret = sdio_io_rw_extended_block(func, 0, addr, 1, (uint8_t *)&dmabuf, 2);
    if (ret)
    {
        if (err)
            *err = ret;
    }

    return (uint16_t)dmabuf;
}

int32_t sdio_io_writew(sdio_function *func,
                          uint16_t              data,
                          uint32_t              addr)
{
    uint32_t dmabuf = data;

    return sdio_io_rw_extended_block(func, 1, addr, 1, (uint8_t *)&dmabuf, 2);
}

uint32_t sdio_io_readl(sdio_function *func,
                          uint32_t              addr,
                          int32_t              *err)
{
    int32_t ret;
    uint32_t dmabuf;

    if (err)
        *err = 0;

    ret = sdio_io_rw_extended_block(func, 0, addr, 1, (uint8_t *)&dmabuf, 4);
    if (ret)
    {
        if (err)
            *err = ret;
    }

    return dmabuf;
}

int32_t sdio_io_writel(sdio_function *func,
                          uint32_t              data,
                          uint32_t              addr)
{
    uint32_t dmabuf = data;

    return sdio_io_rw_extended_block(func, 1, addr, 1, (uint8_t *)&dmabuf, 4);
}

int32_t sdio_io_read_multi_fifo_b(sdio_function *func,
                                     uint32_t              addr,
                                     uint8_t              *buf,
                                     uint32_t              len)
{
    return sdio_io_rw_extended_block(func, 0, addr, 0, buf, len);
}

int32_t sdio_io_write_multi_fifo_b(sdio_function *func,
                                      uint32_t              addr,
                                      uint8_t              *buf,
                                      uint32_t              len)
{
    return sdio_io_rw_extended_block(func, 1, addr, 0, buf, len);
}

int32_t sdio_io_read_multi_incr_b(sdio_function *func,
                                     uint32_t              addr,
                                     uint8_t              *buf,
                                     uint32_t              len)
{
    return sdio_io_rw_extended_block(func, 0, addr, 1, buf, len);
}

int32_t sdio_io_write_multi_incr_b(sdio_function *func,
                                      uint32_t              addr,
                                      uint8_t              *buf,
                                      uint32_t              len)
{
    return sdio_io_rw_extended_block(func, 1, addr, 1, buf, len);
}

static int32_t sdio_read_cccr(sdmmc_card *card)
{
    int32_t ret;
    int32_t cccr_version;
    uint8_t data;

    memset(&card->cccr, 0, sizeof(sdio_cccr));

    data = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_CCCR_REV, &ret);
    if (ret)
        goto out;

    cccr_version = data & 0x0f;

    if (cccr_version > SDIO_CCCR_REV_3_00)
    {
        aicx_error(0x0b8903, "d", "unrecognised CCCR structure version %d", cccr_version);

        return -EFAULT;
    }

    card->cccr.sdio_version = (data & 0xf0) >> 4;

    data = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_CARD_CAPS, &ret);
    if (ret)
        goto out;

    if (data & SDIO_CCCR_CAP_SMB)
        card->cccr.multi_block = 1;
    if (data & SDIO_CCCR_CAP_LSC)
        card->cccr.low_speed = 1;
    if (data & SDIO_CCCR_CAP_4BLS)
        card->cccr.low_speed_4 = 1;
    if (data & SDIO_CCCR_CAP_4BLS)
        card->cccr.bus_width = 1;

    if (cccr_version >= SDIO_CCCR_REV_1_10)
    {
        data = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_POWER_CTRL, &ret);
        if (ret)
            goto out;

        if (data & SDIO_POWER_SMPC)
            card->cccr.power_ctrl = 1;
    }

    if (cccr_version >= SDIO_CCCR_REV_1_20)
    {
        data = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_SPEED, &ret);
        if (ret)
            goto out;

        if (data & SDIO_SPEED_SHS)
            card->cccr.high_speed = 1;
    }

out:
    return ret;
}

static int32_t cistpl_funce_func0(sdmmc_card *card,
                                     const uint8_t     *buf,
                                     uint32_t           size)
{
    if (size < 0x04 || buf[0] != 0)
        return -EFAULT;

    /* TPLFE_FN0_BLK_SIZE */
    card->cis.func0_blk_size = buf[1] | (buf[2] << 8);

    /* TPLFE_MAX_TRAN_SPEED */
    card->cis.max_tran_speed = speed_value[(buf[3] >> 3) & 15] *
                speed_unit[buf[3] & 7];

    return 0;
}

static int32_t cistpl_funce_func(sdio_function *func,
                                    const uint8_t        *buf,
                                    uint32_t              size)
{
    uint32_t version;
    uint32_t min_size;

    version = func->card->cccr.sdio_version;
    min_size = (version == SDIO_SDIO_REV_1_00) ? 28 : 42;

    if (size < min_size || buf[0] != 1)
        return -EFAULT;

    /* TPLFE_MAX_BLK_SIZE */
    func->max_blk_size = buf[12] | (buf[13] << 8);

    /* TPLFE_ENABLE_TIMEOUT_VAL, present in ver 1.1 and above */
    if (version > SDIO_SDIO_REV_1_00)
        func->enable_timeout_val = (buf[28] | (buf[29] << 8)) * 10;
    else
        func->enable_timeout_val = 1000; /* 1000ms */

    return 0;
}

static int32_t sdio_read_cis(sdio_function *func)
{
    int32_t ret;
    sdio_function_tuple *curr, **prev;
    uint32_t i, cisptr = 0;
    uint8_t data;
    uint8_t tpl_code, tpl_link;

    sdmmc_card *card = func->card;
    sdio_function *func0 = card->sdio_function[0];

    AOS_ASSERT(func0 != NULL);

    for (i = 0; i < 3; i++)
    {
        data = sdio_io_readb(func0,
            SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_CIS + i, &ret);
        if (ret)
            return ret;
        cisptr |= data << (i * 8);
    }

    prev = &func->tuples;

    do {
        tpl_code = sdio_io_readb(func0, cisptr++, &ret);
        if (ret)
            break;
        tpl_link = sdio_io_readb(func0, cisptr++, &ret);
        if (ret)
            break;

        if ((tpl_code == CISTPL_END) || (tpl_link == 0xff))
            break;

        if (tpl_code == CISTPL_NULL)
            continue;


        curr = aos_malloc(sizeof(sdio_function_tuple) + tpl_link);
        if (!curr)
            return -ENOMEM;
        curr->data = (uint8_t *)curr + sizeof(sdio_function_tuple);

        for (i = 0; i < tpl_link; i++)
        {
            curr->data[i] = sdio_io_readb(func0, cisptr + i, &ret);
            if (ret)
                break;
        }
        if (ret)
        {
            aos_free(curr);
            break;
        }

        switch (tpl_code)
        {
        case CISTPL_MANFID:
            if (tpl_link < 4)
            {
                aicx_debug(0x0e69be, "", "bad CISTPL_MANFID length");
            }
            else
            {
                if (func->num != 0)
                {
                    func->manufacturer = curr->data[0];
                    func->manufacturer |= curr->data[1] << 8;
                    func->product = curr->data[2];
                    func->product |= curr->data[3] << 8;
                }
                else
                {
                    card->cis.manufacturer = curr->data[0];
                    card->cis.manufacturer |= curr->data[1] << 8;
                    card->cis.product = curr->data[2];
                    card->cis.product |= curr->data[3] << 8;
                }
            }

            aos_free(curr);
            break;
        case CISTPL_FUNCE:
            if (func->num != 0)
                ret = cistpl_funce_func(func, curr->data, tpl_link);
            else
                ret = cistpl_funce_func0(card, curr->data, tpl_link);

            if (ret)
            {
                aicx_debug(0x1b181e, "uu", "bad CISTPL_FUNCE size %u "
                       "type %u", tpl_link, curr->data[0]);
            }

            break;
        case CISTPL_VERS_1:
            if (tpl_link < 2)
            {
                aicx_debug(0x0aa8f2, "", "CISTPL_VERS_1 too short");
            }
            break;
        default:
            /* this tuple is unknown to the core */
            curr->next = NULL;
            curr->code = tpl_code;
            curr->size = tpl_link;
            *prev = curr;
            prev = &curr->next;
            aicx_debug(0x12b6b0, "dxd", "function %d, CIS tuple code %#x, length %d",
                func->num, tpl_code, tpl_link);
            break;
        }

        cisptr += tpl_link;
    } while (1);

    /*
     * Link in all unknown tuples found in the common CIS so that
     * drivers don't have to go digging in two places.
     */
    if (func->num != 0)
        *prev = func0->tuples;

    return ret;
}


void sdio_free_cis(sdio_function *func)
{
    sdio_function_tuple *tuple, *tmp;
    sdmmc_card *card = func->card;

    tuple = func->tuples;

    while (tuple && ((tuple != card->sdio_function[0]->tuples) || (!func->num)))
    {
        tmp = tuple;
        tuple = tuple->next;
        aos_free(tmp);
    }

    func->tuples = NULL;
}

static int32_t sdio_read_fbr(sdio_function *func)
{
    int32_t ret;
    uint8_t data;
    sdio_function *func0 = func->card->sdio_function[0];

    data = sdio_io_readb(func0,
        SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_STD_FUNC_IF, &ret);
    if (ret)
        goto err;

    data &= 0x0f;

    if (data == 0x0f)
    {
        data = sdio_io_readb(func0,
            SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_STD_IF_EXT, &ret);
        if (ret)
            goto err;
    }

    func->func_code = data;

err:
    return ret;
}

static int32_t sdio_initialize_function(sdmmc_card *card,
                                           uint32_t           func_num)
{
    int32_t ret;
    sdio_function *func;

    AOS_ASSERT(func_num <= SDIO_MAX_FUNCTIONS);

    func = aos_malloc(sizeof(sdio_function));
    if (!func)
    {
        aicx_error(0x037239, "", "malloc sdio_function failed");
        ret = -ENOMEM;
        goto err;
    }
    memset(func, 0, sizeof(sdio_function));

    func->card = card;
    func->num = func_num;

    ret = sdio_read_fbr(func);
    if (ret)
        goto err1;

    ret = sdio_read_cis(func);
    if (ret)
        goto err1;

    /*
     * product/manufacturer id is optional for function CIS, so
     * copy it from the card structure as needed.
     */
    if (func->product == 0)
    {
        func->manufacturer = card->cis.manufacturer;
        func->product = card->cis.product;
    }

    card->sdio_function[func_num] = func;

    return 0;

err1:
    sdio_free_cis(func);
    aos_free(func);
    card->sdio_function[func_num] = NULL;
err:
    return ret;
}

static int32_t sdio_set_highspeed(sdmmc_card *card)
{
    int32_t ret;
    uint8_t speed;

    if (!(card->host->flags & SDMMC_SUP_HIGHSPEED))
        return 0;

    if (!card->cccr.high_speed)
        return 0;

    speed = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_SPEED, &ret);
    if (ret)
        return ret;

    speed |= SDIO_SPEED_EHS;

    ret = sdio_io_writeb(card->sdio_function[0], SDIO_REG_CCCR_SPEED, speed);
    if (ret)
        return ret;

    card->flags |= CARD_FLAG_HIGHSPEED;

    return 0;
}

static int32_t sdio_set_bus_wide(sdmmc_card *card)
{
    int32_t ret;
    uint8_t busif;

    if (!(card->host->flags & SDMMC_BUSWIDTH_4))
        return 0;

    if (card->cccr.low_speed && !card->cccr.bus_width)
        return 0;

    busif = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_BUS_IF, &ret);
    if (ret)
        return ret;

    busif |= SDIO_BUS_WIDTH_4BIT;

    ret = sdio_io_writeb(card->sdio_function[0], SDIO_REG_CCCR_BUS_IF, busif);
    if (ret)
        return ret;

    sdmmc_set_bus_width(card->host, SDMMC_BUS_WIDTH_4);

    return 0;
}

int32_t sdio_match_card(sdmmc_card           *card,
                                     const sdio_device_id *id)
{
    uint8_t num = 1;

    if ((id->manufacturer != SDIO_ANY_MAN_ID) &&
        (id->manufacturer != card->cis.manufacturer))
        return 0;

    while (num <= card->sdio_function_num)
    {
        if ((id->product != SDIO_ANY_PROD_ID) &&
            (id->product == card->sdio_function[num]->product))
            return 1;
        num++;
    }

    return 0;
}

int32_t sdio_register_card(sdmmc_card *card)
{
    sdio_card *sc;
    sdio_driver *sd;
    struct list_head *l;

    sc = aos_malloc(sizeof(sdio_card));
    if (sc == NULL)
    {
        aicx_error(0x00f8c3, "", "malloc sdio card failed");
        return -ENOMEM;
    }

    sc->card = card;
    list_add(&sc->list, &sdio_cards);

    if (list_empty(&sdio_drivers))
    {
        goto out;
    }

    for (l = (&sdio_drivers)->next; l != &sdio_drivers; l = l->next)
    {
        sd = (sdio_driver *)list_entry(l, sdio_driver, list);
        if (sdio_match_card(card, sd->drv->id))
        {
            sd->drv->probe(card);
        }
    }

out:
    return 0;
}

int32_t sdio_init_card(sdmmc_host *host, uint32_t ocr)
{
    int32_t err = 0;
    int32_t i, function_num;
    uint32_t  cmd5_resp;
    sdmmc_card *card;

    err = sdio_io_send_op_cond(host, ocr, &cmd5_resp);
    if (err)
        goto err;

    if (controller_is_spi(host))
    {
        err = sdmmc_spi_use_crc(host, host->spi_use_crc);
        if (err)
            goto err;
    }

    function_num = (cmd5_resp & 0x70000000) >> 28;

    card = aos_malloc(sizeof(sdmmc_card));
    if (!card)
    {
        aicx_error(0x08b3f9, "", "malloc card failed");
        err = -ENOMEM;
        goto err;
    }
    memset(card, 0, sizeof(sdmmc_card));

    card->card_type = CARD_TYPE_SDIO;
    card->sdio_function_num = function_num;
    card->host = host;
    host->card = card;

    card->sdio_function[0] = aos_malloc(sizeof(sdio_function));
    if (!card->sdio_function[0])
    {
        aicx_error(0x0ef7d2, "", "malloc sdio_func0 failed");
        err = -ENOMEM;
        goto err1;
    }
    memset(card->sdio_function[0], 0, sizeof(sdio_function));
    card->sdio_function[0]->card = card;
    card->sdio_function[0]->num = 0;

    if (!controller_is_spi(host))
    {
        err = sdmmc_get_card_addr(host, &card->rca);
        if (err)
            goto err2;

        //sdmmc_set_bus_mode(host, SDMMC_BUSMODE_PUSHPULL);
    }

    if (!controller_is_spi(host))
    {
        err = sdmmc_select_card(card);
        if (err)
            goto err2;
    }

    err = sdio_read_cccr(card);
    if (err)
        goto err2;

    err = sdio_read_cis(card->sdio_function[0]);
    if (err)
        goto err2;

    err = sdio_set_highspeed(card);
    if (err)
        goto err2;

    if (card->flags & CARD_FLAG_HIGHSPEED)
    {
        sdmmc_set_clock(host, host->freq_max > 50000000 ? 50000000 : host->freq_max);
    }
    else
    {
        sdmmc_set_clock(host, card->cis.max_tran_speed);
    }

    err = sdio_set_bus_wide(card);
    if (err)
        goto err2;

    for (i = 1; i < function_num + 1; i++)
    {
        err = sdio_initialize_function(card, i);
        if (err)
            goto err3;
    }

    /* register sdio card */
    err = sdio_register_card(card);
    if (err)
    {
        goto err3;
    }

    return 0;

err3:
    if (card)
    {
        for (i = 1; i < card->sdio_function_num + 1; i++)
        {
            if (card->sdio_function[i])
            {
                sdio_free_cis(card->sdio_function[i]);
                aos_free(card->sdio_function[i]);
                card->sdio_function[i] = NULL;
            }
        }
    }
err2:
    if (card && card->sdio_function[0])
    {
        sdio_free_cis(card->sdio_function[0]);
        aos_free(card->sdio_function[0]);
        card->sdio_function[0] = NULL;
    }
err1:
    if (host->card)
    {
        aos_free(host->card);
    }
err:
    aicx_error(0x019033, "d", "error %d while initialising SDIO card", err);

    return err;
}

int32_t init_sdio(sdmmc_host *host, uint32_t ocr)
{
    int32_t err;
    uint32_t  current_ocr;

    AOS_ASSERT(host != NULL);

    if (ocr & 0x7F)
    {
        aicx_warn(0x17292c, "", "Card ocr below the defined voltage rang.");
        ocr &= ~0x7F;
    }

    if (ocr & VDD_165_195)
    {
        aicx_warn(0x03d3f0, "", "Can't support the low voltage SDIO card.");
        ocr &= ~VDD_165_195;
    }

    current_ocr = sdmmc_select_voltage(host, ocr);

    if (!current_ocr)
    {
        err = -EFAULT;
        goto err;
    }

    err = sdio_init_card(host, current_ocr);
    if (err)
        goto remove_card;

    ssv_wifi_init();

    return 0;

remove_card:
    aos_free(host->card);
    host->card = NULL;
err:

    aicx_error(0x033106, "", "init SDIO card failed");

    return err;
}

static void sdio_irq_thread(void *param)
{
    int32_t i, ret;
    uint8_t pending;
    sdmmc_card *card;
    sdmmc_host *host = (sdmmc_host *)param;
    AOS_ASSERT(host != NULL);
    card = host->card;
    AOS_ASSERT(card != NULL);

    while (1)
    {
        if (aos_get_semaphore(host->sdio_irq_sem, AOS_WAIT_FOREVER) == AOS_SUCCESS)
        {
            sdmmc_host_lock(host);
            //USER_TRACE_IN_LINE_INSERT(5000, 0xAA, 0x01);
            //alog_info("sdio_irq_thread enter!\r\n");
            pending = sdio_io_readb(card->sdio_function[0],
                        SDIO_REG_CCCR_INT_PEND, &ret);
            if (ret)
            {
                aicx_info(0x0ac0f9, "d", "error %d reading SDIO_REG_CCCR_INT_PEND\r\n", ret);
                goto out;
            }

            //alog_info("sdio_irq_thread pending: 0x%X\r\n", pending);
            //USER_TRACE_IN_LINE_INSERT(5000, 0xAA, 0x02);
            for (i = 1; i <= 7; i++)
            {
                if (pending & (1 << i))
                {
                    sdio_function *func = card->sdio_function[i];
                    if (!func)
                    {
                        aicx_info(0x15f80e, "d", "pending IRQ for non-existant function %d\r\n", func->num);
                        goto out;
                    }
                    else if (func->irq_handler)
                    {
                        //alog_info("sdio_irq_thread call irq_handler!\r\n");
                        func->irq_handler(func);
                    }
                    else
                    {
                        //alog_info("pending IRQ with no register handler\r\n");
                        goto out;
                    }
                }
            }

        out:
            //USER_TRACE_IN_LINE_INSERT(5000, 0xAA, 0x03);
            sdmmc_host_unlock(host);
            //alog_info("sdio_irq_thread finished!\r\n");
#if 0
            if (host->flags & SDMMC_SUP_SDIO_IRQ)
                host->ops->enable_sdio_irq(host, 1);
#endif
            continue;
        }
    }
}

static int32_t sdio_irq_thread_create(sdmmc_card *card)
{
    sdmmc_host *host = card->host;

    /* init semaphore and create sdio irq processing thread */
    if (!host->sdio_irq_num)
    {
        host->sdio_irq_num++;
        host->sdio_irq_sem = aos_create_semaphore("sdio_irq_sem", 0);
        AOS_ASSERT(host->sdio_irq_sem);

        host->sdio_irq_thread = aos_create_thread_simply("sdio_irq", sdio_irq_thread, host,
                             SDIO_STACK_SIZE, SDIO_THREAD_PRIORITY);
        AOS_ASSERT(host->sdio_irq_thread);
    }

    return 0;
}

static int32_t sdio_irq_thread_delete(sdmmc_card *card)
{
    sdmmc_host *host = card->host;

    AOS_ASSERT(host->sdio_irq_num > 0);

    host->sdio_irq_num--;
    if (!host->sdio_irq_num)
    {
        if (host->flags & SDMMC_SUP_SDIO_IRQ)
            host->ops->enable_sdio_irq(host, 0);
        aos_delete_semaphore(host->sdio_irq_sem);
        host->sdio_irq_sem = NULL;
        aos_delete_thread(host->sdio_irq_thread);
        host->sdio_irq_thread = 0;
    }

    return 0;
}

int32_t sdio_attach_irq(sdio_function *func,
                           sdio_irq_handler_t   *handler)
{
    int32_t ret;
    uint8_t reg;
    sdio_function *func0;

    AOS_ASSERT(func != NULL);
    AOS_ASSERT(func->card != NULL);

    func0 = func->card->sdio_function[0];

    aicx_info(0x0414fd, "d", "SDIO: enabling IRQ for function %d\n", func->num);

    if (func->irq_handler)
    {
        aicx_info(0x1aa67e, "", "SDIO: IRQ for already in use.\n");

        return -EBUSY;
    }

    reg = sdio_io_readb(func0, SDIO_REG_CCCR_INT_EN, &ret);
    if (ret)
        return ret;

    reg |= 1 << func->num;

    reg |= 1; /* Master interrupt enable */

    ret = sdio_io_writeb(func0, SDIO_REG_CCCR_INT_EN, reg);
    if (ret)
        return ret;

    func->irq_handler = handler;

    ret = sdio_irq_thread_create(func->card);
    if (ret)
        func->irq_handler = NULL;

    return ret;
}

int32_t sdio_detach_irq(sdio_function *func)
{
    int32_t ret;
    uint8_t reg;
    sdio_function *func0;

    AOS_ASSERT(func != NULL);
    AOS_ASSERT(func->card != NULL);

    func0 = func->card->sdio_function[0];

    aicx_info(0x142490, "d", "SDIO: disabling IRQ for function %d\n", func->num);

    if (func->irq_handler)
    {
        func->irq_handler = NULL;
        sdio_irq_thread_delete(func->card);
    }

    reg = sdio_io_readb(func0, SDIO_REG_CCCR_INT_EN, &ret);
    if (ret)
        return ret;

    reg &= ~(1 << func->num);

    /* Disable master interrupt with the last function interrupt */
    if (!(reg & 0xFE))
        reg = 0;

    ret = sdio_io_writeb(func0, SDIO_REG_CCCR_INT_EN, reg);
    if (ret)
        return ret;

    return 0;
}

void sdio_irq_wakeup(sdmmc_host *host)
{
    if (host->flags & SDMMC_SUP_SDIO_IRQ)
        host->ops->enable_sdio_irq(host, 0);
    if (host->sdio_irq_sem) {
        //alog_info("sdio int coming! sdio_irq_wakeup to put sem!\r\n");
        //USER_TRACE_IN_LINE_INSERT(5000, 0xAA, 0x00);
        aos_put_semaphore(host->sdio_irq_sem);
    }
}

void aos_sdio_irq_enable(sdio_function *func, boolean enable)
{
    sdmmc_host *host;
    sdio_function *func0;

    func0 = func->card->sdio_function[0];
    host = func0->card->host;

    if (host->flags & SDMMC_SUP_SDIO_IRQ)
        host->ops->enable_sdio_irq(host, enable);

    //alog_info("aos_sdio_irq_enable called! enable: %d, SRS14: 0x%X, SRS14: 0x%X\r\n", enable, AIC_READ32(SD_REG_SRS13), AIC_READ32(SD_REG_SRS14));
}

int32_t sdio_enable_func(sdio_function *func)
{
    int32_t ret;
    uint8_t reg;
    uint32_t timeout;
    sdio_function *func0;

    AOS_ASSERT(func != NULL);
    AOS_ASSERT(func->card != NULL);

    func0 = func->card->sdio_function[0];

    aicx_info(0x1af306, "d", "SDIO: enabling function %d\n", func->num);

    reg = sdio_io_readb(func0, SDIO_REG_CCCR_IO_EN, &ret);
    if (ret)
        goto err;

    reg |= 1 << func->num;

    ret = sdio_io_writeb(func0, SDIO_REG_CCCR_IO_EN, reg);
    if (ret)
        goto err;

    timeout = (uint32_t)aic_get_tick() + func->enable_timeout_val;

    while (1)
    {
        reg = sdio_io_readb(func0, SDIO_REG_CCCR_IO_RDY, &ret);
        if (ret)
            goto err;
        if (reg & (1 << func->num))
            break;
        ret = -ETIME;
        if ((uint32_t)aic_get_tick() > timeout)
            goto err;
    }

    aicx_info(0x09a429, "", "SDIO: enabled function successfull\n");

    return 0;

err:
    aicx_info(0x1ce8c3, "d", "SDIO: failed to enable function %d\n", func->num);
    return ret;
}

int32_t sdio_disable_func(sdio_function *func)
{
    int32_t ret;
    uint8_t reg;
    sdio_function *func0;

    AOS_ASSERT(func != NULL);
    AOS_ASSERT(func->card != NULL);

    func0 = func->card->sdio_function[0];

    aicx_info(0x0445f2, "d", "SDIO: disabling function %d\n", func->num);

    reg =  sdio_io_readb(func0, SDIO_REG_CCCR_IO_EN, &ret);
    if (ret)
        goto err;

    reg &= ~(1 << func->num);

    ret = sdio_io_writeb(func0, SDIO_REG_CCCR_IO_EN, reg);
    if (ret)
        goto err;

    aicx_info(0x04d96a, "", "SDIO: disabled function successfull\n");

    return 0;

err:
    aicx_info(0x0d059f, "d", "SDIO: failed to disable function %d\n", func->num);
    return -EIO;
}

void sdio_set_drvdata(sdio_function *func, void *data)
{
    func->priv = data;
}

void* sdio_get_drvdata(sdio_function *func)
{
    return func->priv;
}

int32_t sdio_set_block_size(sdio_function *func,
                               uint32_t              blksize)
{
    int32_t ret;
    sdio_function *func0 = func->card->sdio_function[0];
    sdmmc_host *host = func->card->host;

    if (blksize > host->max_blk_size)
        return -EFAULT;

    if (blksize == 0)
    {
        blksize = MIN(func->max_blk_size, host->max_blk_size);
        blksize = MIN(blksize, 512u);
    }

    ret = sdio_io_writeb(func0, SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_BLKSIZE,
                 blksize & 0xff);
    if (ret)
        return ret;
    ret = sdio_io_writeb(func0, SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_BLKSIZE + 1,
                 (blksize >> 8) & 0xff);
    if (ret)
        return ret;
    func->cur_blk_size = blksize;

    return 0;
}

static sdmmc_card *sdio_match_driver(sdio_device_id *id)
{
    struct list_head *l;
    sdio_card *sc;
    sdmmc_card *card;

    for (l = (&sdio_cards)->next; l != &sdio_cards; l = l->next)
    {
        sc = (sdio_card *)list_entry(l, sdio_card, list);
        card = sc->card;

        if (sdio_match_card(card, id))
        {
            return card;
        }
    }

    return NULL;
}

int32_t sdio_register_driver(_sdio_driver *driver)
{
    sdio_driver *sd;
    sdmmc_card *card;

    sd = aos_malloc(sizeof(sdio_driver));
    if (sd == NULL)
    {
        aicx_error(0x053d1d, "", "malloc sdio driver failed");

        return -ENOMEM;
    }

    sd->drv = driver;
    list_add(&sd->list, &sdio_drivers);

    if (!list_empty(&sdio_cards))
    {
        card = sdio_match_driver(driver->id);
        if (card != NULL)
        {
            return driver->probe(card);
        }
    }

    return -EFAULT;
}

int32_t sdio_unregister_driver(_sdio_driver *driver)
{
    struct list_head *l;
    sdio_driver *sd = NULL;
    sdmmc_card *card;

    for (l = (&sdio_drivers)->next; l != &sdio_drivers; l = l->next)
    {
        sd = (sdio_driver *)list_entry(l, sdio_driver, list);
        if (sd->drv != driver)
        {
            sd = NULL;
        }
    }

    if (sd == NULL)
    {
        aicx_error(0x015065, "s", "SDIO driver %s not register", driver->name);
        return -EFAULT;
    }

    if (!list_empty(&sdio_cards))
    {
        card = sdio_match_driver(driver->id);
        if (card != NULL)
        {
            driver->remove(card);
            list_del(&sd->list);
            aos_free(sd);
        }
    }

    return 0;
}

void sdio_init(void)
{

}

void sdhost_triger_scan(void)
{

}

