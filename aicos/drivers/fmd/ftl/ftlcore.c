/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the flash translation layer driver.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0d2eee
#define LOG_MODULE    AMOD_FLASH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <ftl_partition_config.h>
#include <ftl_partition_size.h>

#include "fmdcore.h"
#include "ftlcore.h"

extern UINT _lx_nor_flash_adapter_initialize(LX_NOR_FLASH *nor_flash,
                                             CHAR *name);
extern UINT _lx_nor_flash_adapter_export(LX_NOR_FLASH *nor_flash, u8 *dst);
extern UINT _lx_nor_flash_adapter_import(LX_NOR_FLASH *nor_flash, u8 *src);

static LIST_HEAD(s_ftl_dev_list);

static u32 ftl_get_dev_num(void)
{
    return sizeof(s_ftl_table)/sizeof(struct ftl_part_table);
}

static char* ftl_get_dev_name(int devno)
{
    return s_ftl_table[devno].name;
}

static const struct ftl_part_cfg *ftl_get_part_cfg(int devno, int partno)
{
    return &(s_ftl_table[devno].part_cfg[partno]);
}

static int ftl_get_num_of_parts(int devno)
{
    return (s_ftl_table[devno].num);
}

static struct ftl_pdev *ftl_allocate_partition(struct ftl_dev *dev,
                                               const struct ftl_part_cfg *part,
                                               u32 cur_offset)
{
    struct ftl_pdev *pdev;

    pdev = (struct ftl_pdev *)aos_malloc(sizeof(struct ftl_pdev));
    if (pdev == NULL)
        return NULL;

    memset((void *)pdev, 0, sizeof(*pdev));

    pdev->name = part->name;
    pdev->sector_num = part->sector_num;
    pdev->size = pdev->sector_num * dev->sector_size;
    pdev->offset = cur_offset;
    pdev->parent = dev;

    list_head_init(&pdev->node);

    return pdev;
}

static void ftl_device_register(struct ftl_dev *dev)
{
    struct ftl_pdev *pdev;
    struct fmd_pdev *f_pdev;
    const struct ftl_part_cfg *part;
    u32 nr_parts, cur_offset = 0;
    int i, devno = dev->devno;

    /* dev list node init */
    list_head_init(&(dev->head));
    list_head_init(&(dev->node));

    /* add dev node to ftl_dev_list */
    list_add_tail(&dev->node, &(s_ftl_dev_list));
    f_pdev = fmd_get_pdev_by_name(dev->name);
    FTL_ASSERT(f_pdev != NULL);

    nr_parts = ftl_get_num_of_parts(devno);
    if ((f_pdev->parent->devno > 0) && (nr_parts == 1)) {
        /* if thera is only one partition in the partition table, the partition
         * size is automatically adjusted to the actual size of the flash. Only
         * work on external flash.
         */
        struct ftl_part_cfg temp_part;
        u32 lx_sectors, ftl_sectors, factor, shift_bits;

        part = ftl_get_part_cfg(devno, 0);

        /* calculate reserved lx sector num */
        lx_sectors = (f_pdev->parent->erasesize / dev->lx_sector_size - 1) * 2;

        /* convert lx sector num to ftl sector num */
        factor = dev->sector_size / dev->lx_sector_size;
        shift_bits = aic_find_first_bit(factor);
        ftl_sectors = (lx_sectors & ~(factor - 1)) >> shift_bits;

        /* save the number of available sectors */
        temp_part.sector_num = dev->sector_num - ftl_sectors;
        temp_part.name = part->name;

        pdev = ftl_allocate_partition(dev,
                                    (const struct ftl_part_cfg *)&temp_part, 0);
        FTL_ASSERT(pdev != NULL);

        list_add_tail(&pdev->node, &(dev->head));
    } else {
        for (i = 0; i < nr_parts; i++) {
            part = ftl_get_part_cfg(devno, i);

            pdev = ftl_allocate_partition(dev, part, cur_offset);
            FTL_ASSERT(pdev != NULL);

            list_add_tail(&pdev->node, &(dev->head));

            cur_offset = pdev->sector_num + pdev->offset;
        }
    }
}

struct ftl_pdev *ftl_get_pdev(char *name)
{
    struct ftl_dev *dev = NULL;
    struct ftl_pdev *pdev = NULL;

    list_for_each_entry(dev, &s_ftl_dev_list, node, __typeof(*dev)) {
        list_for_each_entry(pdev, &(dev->head), node, __typeof(*pdev)) {
            if (strcmp(pdev->name, name) == 0)
                return pdev;
        }
    }

    return NULL;
}

struct ftl_dev *ftl_get_dev(char *name)
{
    struct ftl_dev *dev = NULL;
    char *dev_name;

    list_for_each_entry(dev, &s_ftl_dev_list, node, __typeof(*dev)) {
        if (dev) {
            dev_name = ftl_get_dev_name(dev->devno);
            FTL_ASSERT(dev_name != NULL);
            if (strcmp(dev_name, name) == 0)
                return dev;
        }
    }

    return NULL;
}

int ftl_init(void)
{
    LX_NOR_FLASH *lx_nor;
    struct ftl_dev *dev;
    UINT status, error_code;
    u32 i, dev_num = ftl_get_dev_num();
    char *name;
#ifndef LX_NOR_DISABLE_EXTENDED_CACHE
    u8 *ftl_cache = NULL;
#endif

    lx_nor_flash_initialize();

    for (i = 0; i < dev_num; i++) {
        lx_nor = (LX_NOR_FLASH *)aos_malloc(sizeof(LX_NOR_FLASH));
        FTL_ASSERT(lx_nor != NULL);

        name = ftl_get_dev_name(i);
        FTL_ASSERT(name != NULL);

        status = lx_nor_flash_open(lx_nor, name,
                                    _lx_nor_flash_adapter_initialize);
        if (status == LX_SUCCESS) {
#ifndef LX_NOR_DISABLE_EXTENDED_CACHE
            ULONG ftl_cache_size, extend_cached_size, mapping_bitmap_size, obsolete_count_size;

            /* size unit: byte */
            mapping_bitmap_size = ((lx_nor->lx_nor_flash_total_physical_sectors + 31) / 32) * 4;
            obsolete_count_size = (lx_nor->lx_nor_flash_total_blocks * sizeof(LX_NOR_OBSOLETE_COUNT_CACHE_TYPE) / 4) * 4;
            extend_cached_size = (LX_NOR_EXTENDED_CACHE_SIZE) * (LX_NOR_SECTOR_SIZE) * sizeof(ULONG);
            ftl_cache_size = mapping_bitmap_size + obsolete_count_size + extend_cached_size;
            ftl_cache = aos_malloc(ftl_cache_size);
            FTL_ASSERT(ftl_cache != NULL);

            status = lx_nor_flash_extended_cache_enable(lx_nor,
                                                        (VOID *)ftl_cache,
                                                        ftl_cache_size);
            FTL_ASSERT(status == LX_SUCCESS);
#endif

            if (lx_nor->lx_nor_flash_free_physical_sectors <
                    (lx_nor->lx_nor_flash_physical_sectors_per_block + 1)) {
                status = _lx_nor_flash_defragment(lx_nor);
                FTL_ASSERT(status == LX_SUCCESS);
            }
        } else {
            FTL_ASSERT(status != LX_NO_MEMORY);

            error_code = lx_nor->lx_nor_flash_diagnostic_system_error;
            if (error_code == LX_PARTITION_OPEN_FAILED) {
                aos_free(lx_nor);
                continue;
            } else {
                FTL_ASSERT(0);
            }
        }

        dev = (struct ftl_dev *)aos_malloc(sizeof(struct ftl_dev));
        FTL_ASSERT(dev != NULL);

        dev->sector_size = FTL_SECTOR_SIZE;
        dev->lx_sector_size = LX_NOR_SECTOR_SIZE*sizeof(ULONG);

        if (dev->sector_size == dev->lx_sector_size) {
            dev->sector_num = lx_nor->lx_nor_flash_total_physical_sectors;
        } else {
            u32 factor, shift_bits, lx_sectors;
            factor = dev->sector_size / dev->lx_sector_size;
            shift_bits = aic_find_first_bit(factor);
            lx_sectors = lx_nor->lx_nor_flash_total_physical_sectors;
            dev->sector_num = (lx_sectors & ~(factor - 1)) >> shift_bits;
        }

        dev->size = dev->sector_num * dev->sector_size;
        dev->lx_nor = lx_nor;
        dev->devno = i;
        dev->name = name;
#ifndef LX_NOR_DISABLE_EXTENDED_CACHE
        dev->cache_memory = ftl_cache;
#endif

        ftl_device_register(dev);
    }

    return AIC_SUCCESS;
}

void ftl_uninit(void)
{
    struct ftl_dev *dev = NULL;
    struct ftl_pdev *pdev = NULL;

    while (!(list_empty(&s_ftl_dev_list))) {
        dev = list_first_entry(&s_ftl_dev_list, __typeof(*dev), node);
        if (dev) {
            list_del(&dev->node);

            while (!(list_empty(&(dev->head)))) {
                pdev = list_first_entry(&(dev->head), __typeof(*pdev), node);
                if (pdev) {
                    list_del(&pdev->node);
                    free(pdev);
                    pdev = NULL;
                }
            }

            free(dev->lx_nor);
#ifndef LX_NOR_DISABLE_EXTENDED_CACHE
            free(dev->cache_memory);
#endif
            free(dev);
            dev = NULL;
        }
    }
}

int ftl_export(char *name, u8 *buf)
{
    struct ftl_dev *dev;
    UINT status;

    if (!name || !buf)
        return AIC_EINVAL;

    dev = ftl_get_dev(name);
    if (!dev || !(dev->lx_nor))
        return AIC_EINVAL;

    status = _lx_nor_flash_adapter_export(dev->lx_nor, buf);
    FTL_ASSERT(status == LX_SUCCESS);

    return AIC_SUCCESS;
}

int ftl_import(char *name, u8 *buf)
{
    struct ftl_dev *dev;
    UINT status;

    if (!name || !buf)
        return AIC_EINVAL;

    dev = ftl_get_dev(name);
    if (!dev || !(dev->lx_nor))
        return AIC_EINVAL;

    status = _lx_nor_flash_adapter_import(dev->lx_nor, buf);
    FTL_ASSERT(status == LX_SUCCESS);

    return AIC_SUCCESS;
}
