/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the uid module
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
#include <stdio.h>
#include <trng.h>

u32 aic_encode(u32 orig)
{
    u32 encode;

    if (orig < 10)
        encode = orig + 48;
    else
        encode = orig + 55;

    return encode;
}

u32 get_trng_uid(void)
{
    int ret, data = 0;

    trng_init();
    ret = trng_get_data(&data, 1);
    trng_deinit();

    return data;
}

#define TRNG_UID_FLAG 0x410400
int get_uid(u8 *buf, int *len)
{
    u32 blk_val1, blk_val2, trng_data;
    u32 x, y,wafer_id;
    u32 LOTID_0, LOTID_1, LOTID_2, LOTID_3, LOTID_4, LOTID_5;
    char *p = (char *)buf;
    int ret;

    ret = aic_efuse_read(1, (u32 *)&blk_val1);
    if (ret) {
        log_print("efuse read fail!\n");

        goto out;
    }

    ret = aic_efuse_read(2, (u32 *)&blk_val2);
    if (ret) {
        log_print("efuse read fail!\n");
        goto out;
    }

    if (!blk_val2) {
        trng_data = get_trng_uid();
        blk_val2 = 0x3EFFFFFF & trng_data;
        trng_data = get_trng_uid();
        blk_val1 = TRNG_UID_FLAG | (trng_data & 0x1F);
        aic_efuse_write(1, blk_val1);
        aic_efuse_write(2, blk_val2);
    }

    y = blk_val2 & 0x7F;
    x = (blk_val2 >> 7) & 0x7F;
    wafer_id = (blk_val2 >> 14) & 0x1F;
    LOTID_0 = (blk_val2 >> 19) & 0x3F;
    LOTID_0 = aic_encode(LOTID_0);
    LOTID_1 = (blk_val2 >> 25) & 0x3F;
    LOTID_1 = aic_encode(LOTID_1);
    LOTID_2 = (blk_val1) & 0x3F;
    LOTID_2 = aic_encode(LOTID_2);
    LOTID_3 = (blk_val1 >> 6) & 0x3F;
    LOTID_3 = aic_encode(LOTID_3);
    LOTID_4 = (blk_val1 >> 12) & 0x3F;
    LOTID_4 = aic_encode(LOTID_4);
    LOTID_5 = (blk_val1 >> 18) & 0x3F;
    LOTID_5 = aic_encode(LOTID_5);

    log_print("uid:%c%c%c%c%c%c%02d%03d%03d", LOTID_0, LOTID_1, LOTID_2,
                                             LOTID_3, LOTID_4, LOTID_5,
                                             wafer_id, x, y);
    *len = sprintf(p, "%c%c%c%c%c%c%02d%03d%03d", LOTID_0, LOTID_1, LOTID_2,
                                             LOTID_3, LOTID_4, LOTID_5,
                                             wafer_id, x, y);

out:
    return ret;
}
