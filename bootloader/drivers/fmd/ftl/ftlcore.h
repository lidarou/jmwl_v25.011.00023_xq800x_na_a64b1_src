/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the ftl core drivers.
 */

#ifndef __FTLCORE_H__
#define __FTLCORE_H__

#include <aic_type.h>
#include <assert.h>
#include <core_op.h>
#include <aic_list.h>
#include <lx_api.h>
#include <stdlib.h>

#define FTL_INT_SAVE        AIC_INT_SAVE_AREA
#define FTL_INT_DISABLE     AIC_INT_DISABLE
#define FTL_INT_RESTORE     AIC_INT_RESTORE

struct ftl_dev {
    LX_NOR_FLASH *lx_nor;

    char *name;
    int devno;

    u32 size;           /* available size of total device */
    u32 sector_num;
    u32 sector_size;

    u32 lx_sector_size;

    /* ftl device list node */
    struct list_head node;

    /* ftl partition list head */
    struct list_head head;
};

struct ftl_pdev {
    char *name;

    u32 ref_count;

    u32 offset;
    u32 size;
    u32 sector_num;

    struct list_head node;

    struct ftl_dev *parent;
};

struct ftl_pdev *ftl_get_pdev(char *name);

#endif /* __FTLCORE_H__ */

