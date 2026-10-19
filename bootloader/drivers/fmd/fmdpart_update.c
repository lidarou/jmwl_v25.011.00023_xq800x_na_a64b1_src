/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *
 */

#include <aic_errno.h>
#include <flash_config.h>
#include <flash_mem_config.h>
#include <fmd.h>
#include <fmd_partition_config.h>
#include <stdlib.h>

#include "console/console.h"
#include "download/dload_cmd.h"
#include "fmdcore.h"
#include "fmdpart.h"

#define FMDPT_MAGIC_NUMBER      0x50544142
#define FMDPT_MAGIC_NUMBER_OFS  0x0
#define FMDPT_MAGIC_NUMBER_SIZE 0x4
#define FMDPT_HEADER_CRC_OFS    0x4
#define FMDPT_HEADER_CRC_SIZE   0x4
#define FMDPT_DEVICE_NUM_OFS    0x8
#define FMDPT_DEVICE_NUM_SIZE   0x4

#define FTL_PTABLE_OFS          (SIZE_2K)
#define FTL_PTABLE_SIZE         (SIZE_2K)
#define FTLPT_MAGIC_NUMBER      0x46544C70
#define FTLPT_MAGIC_NUMBER_OFS  0x0
#define FTLPT_MAGIC_NUMBER_SIZE 0x4
#define FTLPT_HEADER_CRC_OFS    0x4
#define FTLPT_HEADER_CRC_SIZE   0x4
#define FTLPT_SECTOR_SZ_OFS     0x8
#define FTLPT_SECTOR_SZ_SIZE    0x4

#define CRC_16_POLYNOMIAL 0x1021
#define CRC_16_L_POLYNOMIAL 0x8000
#define CRC_16_L_SEED 0x80
#define CRC_16_L_OK 0x00

struct fmd_ptable_node {
    u32 offset;
    u32 size;
    u32 crc;
};

static unsigned int fmd_crc_16_calc(unsigned char *buf_ptr, unsigned int len)
{
    unsigned short crc = 0;
    unsigned int i;

    while (len-- != 0) {
        for (i = CRC_16_L_SEED; i != 0; i = i >> 1) {
            if ((crc & CRC_16_L_POLYNOMIAL) != 0) {
                crc = crc << 1;
                crc = crc ^ CRC_16_POLYNOMIAL;
            } else {
                crc = crc << 1;
            }

            if ((*buf_ptr & i) != 0)
                crc = crc ^ CRC_16_POLYNOMIAL;
        }

        buf_ptr++;
    }

    return (crc);
}

boolean fmd_ptable_is_empty(u32 address, u32 size)
{
    u32 i;
    u32 *ptr = (u32 *)address;

    for (i = 0; i < size/4; i++) {
        if (ptr[i] != 0xFFFFFFFF)
            return FALSE;
    }

    return TRUE;
}

int ftl_partition_table_check(void)
{
    u32 ptable_base, ptable_size;

    ptable_base = FLASH0_PTABLE_BASE + FTL_PTABLE_OFS;
    ptable_size = FTL_PTABLE_SIZE;

    if (*((u32 *)ptable_base + FTLPT_MAGIC_NUMBER_OFS) != FTLPT_MAGIC_NUMBER) {
        /* check if this is the first time to write ptable */
        if (fmd_ptable_is_empty(ptable_base, ptable_size))
            return PCK_PTABLE_EMPTY;
        else
            /* the ptable partition is corruption */
            return PCK_PTABLE_CORRUPTION_ERR;
    } else {
        u32 old_pheader_crc, pheader_ofs, pheader_size;
        u32 crc_check, sector_size;

        old_pheader_crc = *((u32 *)(ptable_base + FTLPT_HEADER_CRC_OFS));
        pheader_ofs = ptable_base + FTLPT_SECTOR_SZ_OFS;
        pheader_size = FTLPT_SECTOR_SZ_SIZE;

        /* check data validity of ptable nodes */
        crc_check = fmd_crc_16_calc((u8 *)pheader_ofs, pheader_size);
        log_print("fmd check ftl pheader crc(0x%x), record crc(0x%x)\n",
                    crc_check, old_pheader_crc);
        if (crc_check != old_pheader_crc)
            return PCK_PTABLE_HEADER_CRC_ERR;

        sector_size = *((u32 *)(ptable_base + FTLPT_SECTOR_SZ_OFS));
        log_print("ftl sector size recorded(0x%x), configured(0x%x)\n",
                    sector_size, LX_SECTOR_CONFIG_SIZE);
        if (sector_size != LX_SECTOR_CONFIG_SIZE)
            return PCK_PTABLE_SECTOR_SIZE_NOT_MATCH;
    }

    return 0;
}

int fmd_partition_table_update(void)
{
    u32 ptable_base = FLASH0_PTABLE_BASE, ptable_size = FLASH0_PTABLE_SIZE;
    u32 pno, devnum, dno, new_devnum;
    u32 ptable_update_flag = 0;
    int ret;

    new_devnum = fmd_get_dev_num();

    if (*((u32 *)ptable_base + FMDPT_MAGIC_NUMBER_OFS) != FMDPT_MAGIC_NUMBER) {
        /* check if this is the first time to write ptable */
        if (fmd_ptable_is_empty(ptable_base, ptable_size))
            ptable_update_flag = 1;
        else
            /* the ptable partition is corruption */
            return PCK_PTABLE_CORRUPTION_ERR;
    } else {
        struct fmd_ptable_node *old_pnode;
        u32 old_pheader_crc, old_devnum, pheader_ofs, pheader_size;
        u32 crc_check, pnode_offset;

        old_pheader_crc = *((u32 *)(ptable_base + FMDPT_HEADER_CRC_OFS));
        old_devnum = *((u32 *)(ptable_base + FMDPT_DEVICE_NUM_OFS));
        pheader_ofs = ptable_base + FMDPT_DEVICE_NUM_OFS;
        pheader_size = FMDPT_DEVICE_NUM_SIZE +
                        old_devnum * sizeof(struct fmd_ptable_node);

        /* check data validity of ptable nodes */
        crc_check = fmd_crc_16_calc((u8 *)pheader_ofs, pheader_size);
        log_print("fmd update pheader crc(0x%x), record crc(0x%x)\n",
                    crc_check, old_pheader_crc);
        if (crc_check != old_pheader_crc)
            return PCK_PTABLE_HEADER_CRC_ERR;

        devnum = MIN(old_devnum, new_devnum);
        pnode_offset = FMDPT_MAGIC_NUMBER_SIZE + FMDPT_HEADER_CRC_SIZE
                        + FMDPT_DEVICE_NUM_SIZE;
        old_pnode = (struct fmd_ptable_node *)(ptable_base + pnode_offset);

        /* compare old and new partition table */
        for (dno = 0; dno < devnum; dno++) {
            struct fmd_part_cfg *old_ptable, *new_ptable;
            u32 offset = old_pnode[dno].offset, size = old_pnode[dno].size;
            u32 old_ptable_crc = old_pnode[dno].crc;
            u32 old_part_num, new_part_num, part_num;

            /* check data validity of ptable */
            crc_check = fmd_crc_16_calc((u8 *)(ptable_base + offset), size);
            log_print("fmd update ptable%d crc(0x%x), record crc(0x%x)\n",
                        dno, crc_check, old_ptable_crc);
            if (crc_check != old_ptable_crc)
                return PCK_PTABLE_CRC_ERR;

            /* get new partition table */
            new_ptable = fmd_get_part_table(dno);
            new_part_num = fmd_get_num_of_parts(dno);

            /* get old partition number */
            old_part_num = size / sizeof(*old_ptable);

            if (new_part_num <= old_part_num)
                part_num = new_part_num - 1;
            else
                part_num = old_part_num;

            /* get old partition table */
            old_ptable = (struct fmd_part_cfg *)(ptable_base + offset);

            for (pno = 0; pno < part_num; pno++) {
                if (!strcmp(old_ptable[pno].name, new_ptable[pno].name)) {
                    if (old_ptable[pno].block_num !=
                                            new_ptable[pno].block_num) {
                        ptable_update_flag = 1;
                        break;
                    }
                } else {
                    ptable_update_flag = 1;
                    break;
                }
            }

            /* erase the left partitions */
            if (dno == 0)
                part_num = new_part_num - 1;
            else
                part_num = new_part_num;

            for (; pno < part_num; pno++) {
                u32 tmp_handle, part_size;

                log_print("fmd update erase %s partition\n",
                            new_ptable[pno].name);

                ret = fmd_open(new_ptable[pno].name, &tmp_handle);
                FMD_ASSERT(ret == AIC_SUCCESS);

                ret = fmd_get_partition_size(tmp_handle, &part_size);
                FMD_ASSERT(ret == AIC_SUCCESS);

                ret = fmd_erase(tmp_handle, 0, part_size);
                FMD_ASSERT(ret == AIC_SUCCESS);

                ret = fmd_close(tmp_handle);
                FMD_ASSERT(ret == AIC_SUCCESS);
            }
        }

        if (old_devnum != new_devnum)
            ptable_update_flag = 1;
    }

    /* save new partition table */
    if (ptable_update_flag) {
        struct fmd_ptable_node *new_pnode;
        u32 new_pnodes_size, ptable_offset, handle, crc, magic_number;
        u32 new_pheader_size, sector_size;
        u32 *ptable_header;

        log_print("fmd update ptable\n");

        ret = fmd_open("ptable", &handle);
        FMD_ASSERT(ret == AIC_SUCCESS);

        ret = fmd_erase(handle, 0, ptable_size);
        FMD_ASSERT(ret == AIC_SUCCESS);

        new_pnodes_size = sizeof(*new_pnode) * new_devnum;
        new_pheader_size = FMDPT_DEVICE_NUM_SIZE + new_pnodes_size;
        ptable_header = (u32 *)malloc(new_pheader_size);
        FMD_ASSERT(ptable_header != NULL);

        ptable_offset = FMDPT_MAGIC_NUMBER_SIZE + FMDPT_HEADER_CRC_SIZE
                        + new_pheader_size;

        /* save device number */
        *ptable_header = new_devnum;

        /* skip device number field */
        new_pnode = (struct fmd_ptable_node *)(ptable_header + 1);

        /* save partiton table to flash */
        for (dno = 0; dno < new_devnum; dno++) {
            const struct fmd_part_cfg *new_ptable;
            u32 new_part_num, new_part_size;

            /* get new partition table */
            new_ptable = fmd_get_part_table(dno);
            new_part_num = fmd_get_num_of_parts(dno);
            new_part_size = new_part_num * sizeof(struct fmd_part_cfg);

            new_pnode[dno].offset = ptable_offset;
            new_pnode[dno].size = new_part_size;
            new_pnode[dno].crc = fmd_crc_16_calc((u8 *)new_ptable,
                                                        new_part_size);

            ret = fmd_write(handle, new_pnode[dno].offset, new_pnode[dno].size,
                        (u8 *)new_ptable);
            FMD_ASSERT(ret == AIC_SUCCESS);

            ptable_offset += new_pnode[dno].size;
        }

        /* save partition table nodes to flash */
        ret = fmd_write(handle, FMDPT_DEVICE_NUM_OFS, new_pheader_size,
                        (u8 *)ptable_header);
        FMD_ASSERT(ret == AIC_SUCCESS);

        magic_number = FMDPT_MAGIC_NUMBER;
        crc = fmd_crc_16_calc((u8 *)ptable_header, new_pheader_size);

        /* save ptable header crc value to flash */
        ret = fmd_write(handle, FMDPT_HEADER_CRC_OFS, FMDPT_HEADER_CRC_SIZE,
                        (u8 *)&crc);
        FMD_ASSERT(ret == AIC_SUCCESS);

        /* save magic number to flash */
        ret = fmd_write(handle, FMDPT_MAGIC_NUMBER_OFS, FMDPT_MAGIC_NUMBER_SIZE,
                        (u8 *)&magic_number);
        FMD_ASSERT(ret == AIC_SUCCESS);

        free((void *)ptable_header);

        /* save ftl ptable sector size to flash */
        sector_size = LX_SECTOR_CONFIG_SIZE;
        ret = fmd_write(handle, FTL_PTABLE_OFS + FTLPT_SECTOR_SZ_OFS,
                        FTLPT_SECTOR_SZ_SIZE, (u8 *)(&sector_size));
        FMD_ASSERT(ret == AIC_SUCCESS);

        magic_number = FTLPT_MAGIC_NUMBER;
        crc = fmd_crc_16_calc((u8 *)(&sector_size), FTLPT_SECTOR_SZ_SIZE);

        /* save ftl ptable header crc value to flash */
        ret = fmd_write(handle, FTL_PTABLE_OFS + FTLPT_HEADER_CRC_OFS,
                        FTLPT_HEADER_CRC_SIZE, (u8 *)&crc);
        FMD_ASSERT(ret == AIC_SUCCESS);

        /* save ftl magic number to flash */
        ret = fmd_write(handle, FTL_PTABLE_OFS + FTLPT_MAGIC_NUMBER_OFS,
                        FTLPT_MAGIC_NUMBER_SIZE, (u8 *)&magic_number);
        FMD_ASSERT(ret == AIC_SUCCESS);

        ret = fmd_close(handle);
        FMD_ASSERT(ret == AIC_SUCCESS);
    } else {
        ret = ftl_partition_table_check();

        if (ret == PCK_PTABLE_EMPTY) {
            u32 handle, sector_size, magic_number, crc;

            log_print("fmd update ftl ptable\n");

            ret = fmd_open("ptable", &handle);
            FMD_ASSERT(ret == AIC_SUCCESS);

            /* save ftl ptable sector size to flash */
            sector_size = LX_SECTOR_CONFIG_SIZE;
            ret = fmd_write(handle, FTL_PTABLE_OFS + FTLPT_SECTOR_SZ_OFS,
                            FTLPT_SECTOR_SZ_SIZE, (u8 *)(&sector_size));
            FMD_ASSERT(ret == AIC_SUCCESS);

            magic_number = FTLPT_MAGIC_NUMBER;
            crc = fmd_crc_16_calc((u8 *)(&sector_size), FTLPT_SECTOR_SZ_SIZE);

            /* save ftl ptable header crc value to flash */
            ret = fmd_write(handle, FTL_PTABLE_OFS + FTLPT_HEADER_CRC_OFS,
                            FTLPT_HEADER_CRC_SIZE, (u8 *)&crc);
            FMD_ASSERT(ret == AIC_SUCCESS);

            /* save ftl magic number to flash */
            ret = fmd_write(handle, FTL_PTABLE_OFS + FTLPT_MAGIC_NUMBER_OFS,
                            FTLPT_MAGIC_NUMBER_SIZE, (u8 *)&magic_number);
            FMD_ASSERT(ret == AIC_SUCCESS);

            ret = fmd_close(handle);
            FMD_ASSERT(ret == AIC_SUCCESS);
        } else {
            return ret;
        }
    }

    return 0;
}

int fmd_partition_table_check(void)
{
    u32 ptable_base = FLASH0_PTABLE_BASE, ptable_size = FLASH0_PTABLE_SIZE;
    u32 devnum, dno, new_devnum;
    int ret;

    new_devnum = fmd_get_dev_num();

    if (*((u32 *)ptable_base + FMDPT_MAGIC_NUMBER_OFS) != FMDPT_MAGIC_NUMBER) {
        /* check if this is the first time to write ptable */
        if (fmd_ptable_is_empty(ptable_base, ptable_size))
            return PCK_PTABLE_EMPTY;
        else
            /* the ptable partition is corruption */
            return PCK_PTABLE_CORRUPTION_ERR;
    } else {
        struct fmd_ptable_node *old_pnode;
        u32 old_pheader_crc, old_devnum, pheader_ofs, pheader_size;
        u32 crc_check, pnode_offset, crc_new;

        old_pheader_crc = *((u32 *)(ptable_base + FMDPT_HEADER_CRC_OFS));
        old_devnum = *((u32 *)(ptable_base + FMDPT_DEVICE_NUM_OFS));
        pheader_ofs = ptable_base + FMDPT_DEVICE_NUM_OFS;
        pheader_size = FMDPT_DEVICE_NUM_SIZE +
                        old_devnum * sizeof(struct fmd_ptable_node);

        /* check data validity of ptable nodes */
        crc_check = fmd_crc_16_calc((u8 *)pheader_ofs, pheader_size);
        log_print("fmd check pheader crc(0x%x), record crc(0x%x)\n",
                    crc_check, old_pheader_crc);
        if (crc_check != old_pheader_crc)
            return PCK_PTABLE_HEADER_CRC_ERR;

        devnum = MIN(old_devnum, new_devnum);
        pnode_offset = FMDPT_MAGIC_NUMBER_SIZE + FMDPT_HEADER_CRC_SIZE
                        + FMDPT_DEVICE_NUM_SIZE;
        old_pnode = (struct fmd_ptable_node *)(ptable_base + pnode_offset);

        /* compare old and new partition table */
        for (dno = 0; dno < devnum; dno++) {
            struct fmd_part_cfg *new_ptable;
            u32 offset = old_pnode[dno].offset, size = old_pnode[dno].size;
            u32 old_ptable_crc = old_pnode[dno].crc;
            u32 new_part_num, new_ptable_size;

            /* check data validity of ptable */
            crc_check = fmd_crc_16_calc((u8 *)(ptable_base + offset), size);
            log_print("fmd check ptable%d crc(0x%x), record crc(0x%x)\n",
                        dno, crc_check, old_ptable_crc);
            if (crc_check != old_ptable_crc)
                return PCK_PTABLE_CRC_ERR;

            /* get new partition table */
            new_ptable = fmd_get_part_table(dno);
            new_part_num = fmd_get_num_of_parts(dno);
            new_ptable_size = new_part_num * sizeof(struct fmd_part_cfg);

            crc_new = fmd_crc_16_calc((u8 *)(new_ptable), new_ptable_size);
            log_print("fmd check new ptable%d crc(0x%x)\n", dno, crc_new);
            if (crc_new != crc_check) {
                ret = ftl_partition_table_check();
                if (ret == PCK_PTABLE_SECTOR_SIZE_NOT_MATCH)
                    return ret;
                return PCK_PTABLE_NOT_MATCH;
            }
        }
    }

    /* fmd ptable has not changed, now check ftl ptable */
    ret = ftl_partition_table_check();

    return ret;
}

void fmd_erase_all_chips(void)
{
    u32 dno;
    int ret, dev_num;

    dev_num = fmd_get_dev_num();

    for (dno = 0; dno < dev_num; dno++) {
        log_print("fmd erase device%d\n", dno);
        ret = fmd_erase_total_chip(dno);
        FMD_ASSERT(ret == AIC_SUCCESS);
    }
}

int fmd_get_old_partition_info(char *name, u32 *paddr, u32 *psize)
{
    u32 ptable_base = FLASH0_PTABLE_BASE;
    u32 devnum, dno;
    struct fmd_ptable_node *old_pnode;
    u32 pnode_offset, partition_offset;

    devnum = *((u32 *)(ptable_base + FMDPT_DEVICE_NUM_OFS));
    pnode_offset = FMDPT_MAGIC_NUMBER_SIZE + FMDPT_HEADER_CRC_SIZE
                    + FMDPT_DEVICE_NUM_SIZE;
    old_pnode = (struct fmd_ptable_node *)(ptable_base + pnode_offset);

    /* compare old and new partition table */
    for (dno = 0; dno < devnum; dno++) {
        struct fmd_part_cfg *old_ptable;
        u32 offset = old_pnode[dno].offset, size = old_pnode[dno].size;
        u32 part_num, pno;

        /* get old partition number */
        part_num = size / sizeof(*old_ptable);

        /* get old partition table */
        old_ptable = (struct fmd_part_cfg *)(ptable_base + offset);

        partition_offset = 0;
        for (pno = 0; pno < part_num; pno++) {
            if (!strcmp(old_ptable[pno].name, name)) {
                *psize = old_ptable[pno].block_num * FLASH_BLOCK_SIZE;
                *paddr = (FLASH_BASE + dno * 0x4000000) + partition_offset;
                return 0;
            }

            partition_offset += old_ptable[pno].block_num * FLASH_BLOCK_SIZE;
        }
    }

    return PCK_PTABLE_PARTITION_NOT_FIND_ERR;
}
