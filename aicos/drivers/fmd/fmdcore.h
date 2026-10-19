/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the flash management device drivers.
 */

#ifndef __FMDCORE_H__
#define __FMDCORE_H__

#include <aic_assert.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <aic_type.h>
#include <core_op.h>
#include <fmd.h>

#define FMD_INT_SAVE        AIC_INT_SAVE_AREA
#define FMD_INT_DISABLE     AIC_INT_DISABLE
#define FMD_INT_RESTORE     AIC_INT_RESTORE

#define FMD_ASSERT          AIC_ASSERT
#define FMD_ASSERT_INFO     AIC_ASSERT_INFO
#define FMD_ASSERT_INFX     AICX_ASSERT_IFX

/**
 * FMD partitions access property
 * */
#define FMD_READONLY        0
#define FMD_WRITEABLE       1

struct fmd_part {
    char *name;
    u32 offset;
    u32 size;
    u32 flags;
};

/**
 * struct fmd_pdev - FMD partition device
 *
 */
struct fmd_pdev {
    char *name;

    u32 ref_count;
    u32 id;
    u32 offset;
    u32 size;
    u32 property;

    struct list_head node;

    struct fmd_dev *parent;
};

int fmd_device_register(struct fmd_dev *dev);
struct fmd_pdev *fmd_get_pdev_by_name(char *name);

#endif /* __FMDCORE_H__ */

