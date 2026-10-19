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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x121b4c
#define LOG_MODULE    AMOD_FLASH

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <fmd_partition_config.h>

#include "fmdcore.h"

char* fmd_get_dev_name(u8 devno)
{
    return s_fmd_ptable_array[devno].name;
}

static struct fmd_part_cfg *fmd_get_part_cfg(u8 devno, int partno)
{
    return (struct fmd_part_cfg *)&(s_fmd_ptable_array[devno].part_cfg[partno]);
}

int fmd_get_num_of_parts(u8 devno)
{
    return s_fmd_ptable_array[devno].num;
}

static struct fmd_group_cfg *fmd_get_group_cfg(u8 devno, int gno)
{
    return (struct fmd_group_cfg *)&(s_fmd_ptable_array[devno].group_cfg[gno]);
}

int fmd_get_num_of_groups(u8 devno)
{
    return s_fmd_ptable_array[devno].group_num;
}

int fmd_parse_partitions(struct fmd_dev *dev, int partno,
                            struct fmd_part *part)
{
    struct fmd_part_cfg *part_cfg;

    part_cfg = fmd_get_part_cfg(dev->devno, partno);
    part->name = part_cfg->name;
    part->size = part_cfg->block_num * dev->erasesize;

    if (strcmp(part_cfg->attribute, "RO") == 0)
        part->flags = FMD_READONLY;
    else if (strcmp(part_cfg->attribute, "RW") == 0)
        part->flags = FMD_WRITEABLE;
    else
        return AIC_EINVAL;

    return AIC_SUCCESS;
}

struct fmd_pdev *fmd_allocate_partition(struct fmd_dev *dev,
                                        struct fmd_part *part,
                                        u32 cur_offset)
{
    struct fmd_pdev *pdev;

    pdev = (struct fmd_pdev *)aos_malloc(sizeof(*pdev));
    if (pdev == NULL)
        return NULL;

    memset((void *)pdev, 0, sizeof(*pdev));

    list_head_init(&pdev->node);

    pdev->name = part->name;
    pdev->size = part->size;
    pdev->offset = cur_offset;
    pdev->property = part->flags;

    pdev->parent = dev;

    return pdev;
}

struct fmd_pdev *fmd_allocate_group(struct fmd_dev *dev, u32 gno)
{
    struct fmd_pdev *pdev;
    struct fmd_group_cfg *group;

    pdev = (struct fmd_pdev *)aos_malloc(sizeof(*pdev));
    if (pdev == NULL)
        return NULL;

    memset((void *)pdev, 0, sizeof(*pdev));

    list_head_init(&pdev->node);

    group = fmd_get_group_cfg(dev->devno, gno);
    pdev->name = group->name;
    pdev->size = group->block_num * dev->erasesize;
    pdev->offset = group->offset * dev->erasesize;
    if (strcmp(group->attribute, "RO") == 0)
        pdev->property = FMD_READONLY;
    else if (strcmp(group->attribute, "RW") == 0)
        pdev->property = FMD_WRITEABLE;
    else
        AIC_ASSERT(0);

    pdev->parent = dev;

    return pdev;
}

