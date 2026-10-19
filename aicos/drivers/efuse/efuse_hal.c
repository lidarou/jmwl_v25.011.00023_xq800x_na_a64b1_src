/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of watchdog HAL module
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14901d
#define LOG_MODULE    AMOD_DRV

#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#include "efuse_phy.h"
#include "uid_phy.h"
#include <aic_errno.h>

aos_mutex_ptr aic_efuse_mutex = AOS_NULL;

//struct efuse_blk_info efuse_bitmap[EFUSE_BLK_NUMS];

int aic_efuse_block_write(int blk, u32 blk_val)
{
    int ret = 0;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_efuse_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    ret = aic_efuse_write(blk, blk_val);
    if (ret)
        aicx_error(0x1e0c9d, "", "ewf\n");

    if (aos_is_user_thread_context())
        aos_put_mutex(aic_efuse_mutex);

    return ret;
}

int aic_efuse_block_read(int blk, u32 *blk_val)
{
    u32 ret;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_efuse_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    ret = aic_efuse_read(blk, blk_val);
    if (ret)
        aicx_error(0x0fe48f, "", "erf\n");

    if (aos_is_user_thread_context())
        aos_put_mutex(aic_efuse_mutex);

    return ret;
}

struct bit_array{
    u8 bit;
};

int aic_efuse_block_lock(int blk)
{
    int ret;

    ret = aic_efuse_block_write(blk, BIT(EFUSE_BLK_LOCK_BIT));
    if (ret)
        aicx_error(0x0e4483, "sd", "%s blk %d lock failed!", __func__, blk);

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
            bit_array[bit_array_idx].bit = BIT_0 & (byte_op >> j);
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
        aicx_error(0x16d9d9, "", "fill otp enob fail:size overflow, should separate multiple blocks!!!\n");
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
    bit_array = (struct bit_array *)aos_malloc(bits_len + 1);

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

    ret = aic_efuse_block_write(cur_blk_idx++, otp_blk_data);
    if (ret) {
        aicx_error(0x1afdeb, "d", "blk:%d write fail!\n", (cur_blk_idx - 1));
        goto out;
    }

    /*mid blks fill*/
    for (i = 0; i < mid_blks; i++) {
        ret = fill_single_otp_blk_enob((u32 *)&otp_blk_data, &bit_array[cur_bit_idx], EFUSE_ENOB_LENTH, 0);
        if (ret)
            goto out;

        cur_bit_idx += EFUSE_ENOB_LENTH;

        ret = aic_efuse_block_write(cur_blk_idx++, otp_blk_data | BIT(EFUSE_BLK_LOCK_BIT));
        if(ret) {
            aicx_error(0x1afdeb, "d", "blk:%d write fail!\n", (cur_blk_idx - 1));
            goto out;
        }
    }

    /*rear blk fill*/
    if (rear_bits) {
        ret = fill_single_otp_blk_enob((u32 *)&otp_blk_data, &bit_array[cur_bit_idx], bits_len - cur_bit_idx, 0);
        if (ret)
            goto out;

        cur_bit_idx += bits_len - cur_bit_idx;

        ret = aic_efuse_block_write(cur_blk_idx++, otp_blk_data | BIT(EFUSE_BLK_LOCK_BIT));
        if (ret) {
            aicx_error(0x1afdeb, "d", "blk:%d write fail!\n", (cur_blk_idx - 1));
            goto out;
        }
    }

    /*operate total bits check*/
    if(cur_bit_idx != bits_len) {
        ret = AIC_ENOTSUP;
        aicx_error(0x09c4b1, "dd", "opearte bits can't match total bit len cur_bit_idx:%d bits_len:%d!\n",
        cur_bit_idx, bits_len);
    }

out:
    aos_free((void *)bit_array);

    return ret;
}

static int get_single_otp_blk_enob(u32 otp_data, struct bit_array *bit_array, int bit_len, int otp_blk_bit_idx)

{
    int i;

    if ((otp_blk_bit_idx + bit_len) > EFUSE_ENOB_LENTH ) {
        aicx_error(0x151ec3, "", "get otp_blk_enob fail:size overflow, should separate multiple blocks!!!\n");
        return AIC_ENOTSUP;
    }

    for (i = 0; i < bit_len; i ++) {
        bit_array[i].bit = BIT_0 & (otp_data >> (otp_blk_bit_idx + i));
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
    bit_array = (struct bit_array *)aos_malloc(bits_len + 1);

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
    ret = aic_efuse_block_read(cur_blk_idx++, (u32 *)&otp_blk_data);
    if (ret) {
        aicx_error(0x0fa5c3, "d", "blk:%d read fail!\n", (cur_blk_idx - 1));
        goto out;
    }
    cur_bit_idx = 0;

    ret = get_single_otp_blk_enob(otp_blk_data, &bit_array[cur_bit_idx], front_bits, bit_idx);
    if (ret)
        goto out;

    cur_bit_idx += front_bits;

    /*mid blks fill*/
    for (i = 0; i < mid_blks; i++) {
        ret = aic_efuse_block_read(cur_blk_idx++, (u32 *)&otp_blk_data);
        if(ret) {
            aicx_error(0x1afdeb, "d", "blk:%d write fail!\n", (cur_blk_idx - 1));
            goto out;
        }

        ret = get_single_otp_blk_enob(otp_blk_data, &bit_array[cur_bit_idx], EFUSE_ENOB_LENTH, 0);
        if (ret)
            goto out;

        cur_bit_idx += EFUSE_ENOB_LENTH;
    }

    /*rear blk fill*/
    if (rear_bits) {
        ret = aic_efuse_block_read(cur_blk_idx++, (u32 *)&otp_blk_data);
        if (ret) {
            aicx_error(0x1afdeb, "d", "blk:%d write fail!\n", (cur_blk_idx - 1));
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
        aicx_error(0x09c4b1, "dd", "opearte bits can't match total bit len cur_bit_idx:%d bits_len:%d!\n",
        cur_bit_idx, bits_len);

    }

out:
    aos_free((void *)bit_array);
    return ret;
}

int aic_get_uid(u8 *uid, int *len)
{
    u32 ret;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_efuse_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    ret = get_uid(uid, len);

    if (aos_is_user_thread_context())
        aos_put_mutex(aic_efuse_mutex);

    return ret;
}

int aic_efuse_init(void)
{
    aic_efuse_mutex = aos_create_mutex("ehal");
    AIC_ASSERT(aic_efuse_mutex);
    aic_efuse_phy_probe();
//    aic_efuse_blk_property_scan(efuse_bitmap);

    return 0;
}
AIC_DRV_P1_INIT_EXPORT(aic_efuse_init);
