/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the efuse software driver, it is efuse
 *    r1p0 phy driver.
*/
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aon_glb.h>
#include <top_glb.h>
#include <console/console.h>
#include <efuse.h>
#include <initcall.h>
#include <reg_base.h>
#include <stdlib.h>
#include <timer.h>
#include "./efuse_reg.h"
#include <efuse_cfg.h>

#define BYTE_MASK(x)                                                            (((x) & 0xFF))
#define BIT_MSK(x)                                                              (((x) & 0x1))

struct bit_array{
    u8 bit:1;
};

typedef enum {
    AIC_EFUSE_READ_FINISH = 1,
    AIC_EFUSE_WRITE_FINISH = 2,
    AIC_EFUSE_ALL01_CHECK = 4,
    AIC_EFUSE_EXCEPTION = 8,
    AIC_EFUSE_ALL = 0xF
} AIC_EFUSE_INT;

static void aic_efuse_int_status_clr(AIC_EFUSE_INT efuse_int)
{
    AIC_WRITE32(REG_EFUSE_EFUSE_INT_STATUS_CLR, efuse_int);
}

static int aic_efuse_wait_int_finish(AIC_EFUSE_INT efuse_int)
{
    int i, ret;
    u32 val;

    ret = 0;
    i = 0;

    while( i < EFUSE_WAIT_TIME_CNT) {
        i ++;
        val = AIC_READ32(REG_EFUSE_EFUSE_INT_STATUS) & efuse_int;

        if (val)
            break;
        aic_mdelay(1);
    }

    aic_efuse_int_status_clr(efuse_int);

    if(i == EFUSE_WAIT_TIME_CNT)
        ret = AIC_ENOTSUP;

    return ret;
}

static void aic_efuse_int_enable(AIC_EFUSE_INT efuse_int)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_INT_ENABLE);
    val = val | efuse_int;
    AIC_WRITE32(REG_EFUSE_EFUSE_INT_ENABLE, val);
}

static void aic_efuse_write_unlock(void)
{
    AIC_WRITE32(REG_EFUSE_EFUSE_MAGIC_DATA, BIT_EFUSE_EFUSE_MAGIC_DATA_WR(EFUSE_WRITE_MAGIC));
}

static void aic_efuse_write_lock(void)
{
    AIC_WRITE32(REG_EFUSE_EFUSE_MAGIC_DATA, BIT_EFUSE_EFUSE_MAGIC_DATA_WR(0));
}

static void aic_efuse_blk_rw_trigger(void)
{
    AIC_WRITE32(REG_EFUSE_EFUSE_RW_TRIGGER, BIT_EFUSE_EFUSE_RW_TRIGGER);
}

static void aic_efuse_write_blk_pre(int blk_id, u32 date)
{
    u32 val;

    /*enable write mode & set blk id*/
    val = BIT_EFUSE_EFUSE_ADDR(blk_id) | BIT_EFUSE_EFUSE_WRITE_EN;
    AIC_WRITE32(REG_EFUSE_EFUSE_RW_CTRL, val);
    AIC_WRITE32(REG_EFUSE_EFUSE_WRITE_DATA, date);
}

static void aic_efuse_read_blk_pre(int blk_id)
{
    u32 val;

    /*disable write mode & set blk id*/
    val = BIT_EFUSE_EFUSE_ADDR(blk_id) & ~BIT_EFUSE_EFUSE_WRITE_EN;
    AIC_WRITE32(REG_EFUSE_EFUSE_RW_CTRL, val);
}

static u32 aic_efuse_read_blk_post()
{
    return AIC_READ32(REG_EFUSE_EFUSE_READ_DATA);
}

static void aic_efuse_auto_pd_enabled(bool auto_pd_enabled)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_POWER_CTRL);

    if (auto_pd_enabled)
        AIC_WRITE32(REG_EFUSE_EFUSE_POWER_CTRL, val | BIT_EFUSE_PD_AUTO_EN);
    else
        AIC_WRITE32(REG_EFUSE_EFUSE_POWER_CTRL, val & ~BIT_EFUSE_PD_AUTO_EN);
}

static void aic_efuse_cache_enabled(bool cache_enabled)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_GLB_CTRL);

    if (cache_enabled)
        AIC_WRITE32(REG_EFUSE_EFUSE_GLB_CTRL, val | BIT_EFUSE_EFUSE_CACHE_ENABLE);
    else
        AIC_WRITE32(REG_EFUSE_EFUSE_GLB_CTRL, val & ~BIT_EFUSE_EFUSE_CACHE_ENABLE);
}

static void aic_efuse_enabled(bool efuse_enabled)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_GLB_CTRL);

    if (efuse_enabled)
        AIC_WRITE32(REG_EFUSE_EFUSE_GLB_CTRL, val | BIT_EFUSE_EFUSE_EN);
    else
        AIC_WRITE32(REG_EFUSE_EFUSE_GLB_CTRL, val & ~BIT_EFUSE_EFUSE_EN);
}

static void aic_efuse_aon_enable(void)
{
    AIC_WRITE32(REG_TOP_GLB_EFUSE_EB, BIT_TOP_GLB_EFUSE_EB);
}

int aic_efuse_write(int blk, u32 data)
{
    int ret = 0;

    aic_efuse_int_enable(AIC_EFUSE_ALL);
    aic_efuse_write_unlock();
    log_print("%s(%d) blk:%d data:0x%x\n", __func__, __LINE__, blk, data);
    aic_efuse_write_blk_pre(blk, data);
    aic_efuse_blk_rw_trigger();

    ret = aic_efuse_wait_int_finish(AIC_EFUSE_WRITE_FINISH);
    if (ret) {
        log_print("efuse write fail, timeout!!\n");
    }
    aic_efuse_write_lock();
    aic_efuse_int_status_clr(AIC_EFUSE_ALL);

    return ret;
}

int aic_efuse_read(int blk,u32 *data)
{
    int ret;

    aic_efuse_int_enable(AIC_EFUSE_ALL);
    aic_efuse_read_blk_pre(blk);
    aic_efuse_blk_rw_trigger();

    ret = aic_efuse_wait_int_finish(AIC_EFUSE_READ_FINISH);
    if (ret) {
        log_print("efuse read fail, timeout!!\n");
    }

    *data = aic_efuse_read_blk_post();
    aic_efuse_int_status_clr(AIC_EFUSE_ALL);
    log_print("%s(%d) blk:%d data:0x%x\n", __func__, __LINE__, blk, *data);

    return ret;
}

static void src_convert_bit_array(uint8_t *byte_src, int bytes_num, struct bit_array *bit_array)
{
    uint8_t byte_op;
    int bit_array_idx;

    for (int i = 0; i < bytes_num; i++) {
        byte_op = byte_src[i];

        for (int j = 0; j < 8; j++) {
            bit_array_idx = i * 8 + j;
            bit_array[bit_array_idx].bit = BIT_MSK(byte_op >> j);
        }
    }
}

static void bit_array_convert_src(struct bit_array *bit_array, int bit_nums, uint8_t *byte_src)
{
    int bytes_num;

    if (!(bit_nums % 8))
        bytes_num = bit_nums / 8;
    else
        bytes_num = bit_nums / 8 + 1;

    for (int i = 0; i < bytes_num; i++) {
        for (int j = 0; j < 8; j++) {
            byte_src[i] |= bit_array[i * 8 + j].bit << j;
        }
    }
}

static int fill_single_otp_blk_enob(u32 *otp_data, const struct bit_array *bit_array, int bit_len, int otp_blk_bit_idx)
{
    u32 blk_data = 0;
    int i;

    if ((otp_blk_bit_idx + bit_len) > EFUSE_ENOB_LENTH ) {
        log_print("fill otp enob fail:size overflow, should separate multiple blocks!!!\n");
        return AIC_ENOTSUP;
    }

    for (i = 0; i < bit_len; i ++) {
        blk_data |= bit_array[i].bit << (otp_blk_bit_idx + i);
    }
    *otp_data = blk_data;

    return 0;
}

int aic_write_efuse_enob(uint8_t *src, int blk_idx, int bit_idx, int bits_len)
{
    int front_bits, rear_bits, cur_bit_idx;
    int mid_blks, front_enos_max, cur_blk_idx;
    int i;
    int ret;
    uint32_t otp_blk_data;
    struct bit_array *bit_array;

    /*alloc temp space to store bit array, these bits will write to otp*/
    bit_array = (struct bit_array *)malloc(bits_len / 8 + 1);

    src_convert_bit_array(src, bits_len / 8, bit_array);

    front_enos_max = EFUSE_ENOB_LENTH - bit_idx;

    if (bits_len <= front_enos_max) {
        front_bits = bits_len;
    } else {
        front_bits = front_enos_max;
    }

    mid_blks = (bits_len - front_bits) / EFUSE_ENOB_LENTH;
    rear_bits = bits_len - front_bits - mid_blks * EFUSE_ENOB_LENTH;

    /*front blk fill*/
    cur_bit_idx = 0;
    cur_blk_idx = blk_idx;
    ret = fill_single_otp_blk_enob((u32 *)&otp_blk_data, &bit_array[cur_bit_idx], front_bits, bit_idx);
    if (ret)
        goto out;

    cur_bit_idx += front_bits;

    ret = aic_efuse_write(cur_blk_idx++, otp_blk_data);
    if (ret) {
        log_print("blk:%d write fail!\n", (cur_blk_idx - 1));
        goto out;
    }

    /*mid blks fill*/
    for (i = 0; i < mid_blks; i++) {
        ret = fill_single_otp_blk_enob((u32 *)&otp_blk_data, &bit_array[cur_bit_idx], EFUSE_ENOB_LENTH, 0);
        if (ret)
            goto out;

        cur_bit_idx += EFUSE_ENOB_LENTH;

        ret = aic_efuse_write(cur_blk_idx++, otp_blk_data);
        if(ret) {
            log_print("blk:%d write fail!\n", (cur_blk_idx - 1));
            goto out;
        }
    }

    /*rear blk fill*/
    if (rear_bits) {
        ret = fill_single_otp_blk_enob((u32 *)&otp_blk_data, &bit_array[cur_bit_idx], bits_len - cur_bit_idx, 0);
        if (ret)
            goto out;

        cur_bit_idx += bits_len - cur_bit_idx;

        ret = aic_efuse_write(cur_blk_idx++, otp_blk_data);
        if (ret) {
            log_print("blk:%d write fail!\n", (cur_blk_idx - 1));
            goto out;
        }
    }

    /*operate total bits check*/
    if(cur_bit_idx != bits_len) {
        ret = AIC_ENOTSUP;
        log_print("opearte bits can't match total bit len cur_bit_idx:%d bits_len:%d!\n",
        cur_bit_idx, bits_len);
    }

out:
    free((void *)bit_array);

    return ret;
}

static int get_single_otp_blk_enob(u32 otp_data, struct bit_array *bit_array, int bit_len, int otp_blk_bit_idx)

{
    int i;

    if ((otp_blk_bit_idx + bit_len) > EFUSE_ENOB_LENTH ) {
        log_print("get otp_blk_enob fail:size overflow, should separate multiple blocks!!!\n");
        return AIC_ENOTSUP;
    }

    for (i = 0; i < bit_len; i ++) {
        bit_array[i].bit = BIT_MSK(otp_data >> (otp_blk_bit_idx + i));
    }

    return 0;
}

int aic_read_efuse_enob(uint8_t *dest, int blk_idx, int bit_idx, int bits_len)
{
    int front_bits, rear_bits, cur_bit_idx;
    int mid_blks, front_enos_max, cur_blk_idx;
    int i;
    int ret;
    uint32_t otp_blk_data = 0;
    struct bit_array *bit_array;

    /*alloc temp space to store bit array, these bits will write to otp*/
    bit_array = (struct bit_array *)malloc(bits_len / 8 + 1);

    /*prepare calculate front/mid/rear blk bit info*/
    front_enos_max = EFUSE_ENOB_LENTH - bit_idx;
    if (bits_len <= front_enos_max) {
        front_bits = bits_len;
    } else {
        front_bits = front_enos_max;
    }
    mid_blks = (bits_len - front_bits) / EFUSE_ENOB_LENTH;
    rear_bits = bits_len - front_bits - mid_blks * EFUSE_ENOB_LENTH;

    cur_blk_idx = blk_idx;

    /*front blk fill*/
    ret = aic_efuse_read(cur_blk_idx++, (u32 *)&otp_blk_data);
    if (ret) {
        log_print("blk:%d read fail!\n", (cur_blk_idx - 1));
        goto out;
    }
    cur_bit_idx = 0;

    ret = get_single_otp_blk_enob(otp_blk_data, &bit_array[cur_bit_idx], front_bits, bit_idx);
    if (ret)
        goto out;

    cur_bit_idx += front_bits;

    /*mid blks fill*/
    for (i = 0; i < mid_blks; i++) {
        ret = aic_efuse_read(cur_blk_idx++, (u32 *)&otp_blk_data);
        if(ret) {
            log_print("blk:%d write fail!\n", (cur_blk_idx - 1));
            goto out;
        }

        ret = get_single_otp_blk_enob(otp_blk_data, &bit_array[cur_bit_idx], EFUSE_ENOB_LENTH, 0);
        if (ret)
            goto out;

        cur_bit_idx += EFUSE_ENOB_LENTH;
    }

    /*rear blk fill*/
    if (rear_bits) {
        ret = aic_efuse_read(cur_blk_idx++, (u32 *)&otp_blk_data);
        if (ret) {
            log_print("blk:%d write fail!\n", (cur_blk_idx - 1));
            goto out;
        }

        ret = get_single_otp_blk_enob(otp_blk_data, &bit_array[cur_bit_idx], bits_len - cur_bit_idx, 0);
        if (ret)
            goto out;

        cur_bit_idx += (bits_len - cur_bit_idx);
    }

    /*memset dest array before fill pubkey*/
    memset(dest, 0, bits_len / 8);

    /*bit array convert byte array, and fill target array*/
    bit_array_convert_src(&bit_array[0], bits_len, dest);

    /*operate total bits check*/
    if(cur_bit_idx != bits_len) {
        ret = AIC_ENOTSUP;
        log_print("opearte bits can't match total bit len cur_bit_idx:%d bits_len:%d!\n",
        cur_bit_idx, bits_len);

    }

out:
    free((void *)bit_array);
    return ret;
}

int aic_efuse_init(void)
{
    aic_efuse_aon_enable();
    aic_efuse_enabled(TRUE);
    aic_efuse_auto_pd_enabled(TRUE);
    aic_efuse_cache_enabled(TRUE);

    return 0;
}
early_init(aic_efuse_init);
