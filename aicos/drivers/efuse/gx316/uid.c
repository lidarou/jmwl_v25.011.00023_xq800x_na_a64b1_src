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
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0cf7aa
#define LOG_MODULE    AMOD_DRV

#include<stdio.h>
#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#include "../efuse_phy.h"
#include <aic_errno.h>
#include "../uid_phy.h"

u32 aic_encode(u32 orig)
{
    u32 encode;

    if (orig < 10)
        encode = orig + 48;
    else
        encode = orig + 55;

    return encode;
}

int get_uid(u8 *buf, int *len)
{
    u32 blk_val1, blk_val2;
    u32 x, y,wafer_id;
    u32 LOTID_0, LOTID_1, LOTID_2, LOTID_3, LOTID_4, LOTID_5;
    char *p = buf;
    int ret;

    ret = aic_efuse_read(1, (u32 *)&blk_val1);
    if (ret) {
        aicx_error(0x1d8215, "", "efuse read fail!\n");
        goto out;
    }

    ret = aic_efuse_read(2, (u32 *)&blk_val2);
    if (ret) {
        aicx_error(0x1d8215, "", "efuse read fail!\n");
        goto out;
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

    aicx_warn(0x12b72b, "xx", "chip uid:%x%x", blk_val1, blk_val2);

    *len = sprintf(p, "%c%c%c%c%c%c%02d%03d%03d", LOTID_0, LOTID_1, LOTID_2,
                                             LOTID_3, LOTID_4, LOTID_5,
                                             wafer_id, x, y);
out:
    return ret;
}
