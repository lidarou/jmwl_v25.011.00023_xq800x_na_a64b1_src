/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic nvram device config (API).
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05b678
#define LOG_MODULE    AMOD_NVRAM

#include <stdbool.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <cache.h>
#include <mpu.h>
#include <aic_cmdline.h>
#include <psram_mem_config.h>
#include <sys_mem_api.h>
#endif
#include <aic_nvram_base.h>
#include "aic_nvram_device_config.h"
#if defined(CONFIG_SUBSYS_CP) || defined(WIN32)
#include <aic_nvram_id.h>
#endif

/**-----------------------------nvram share ram--------------------------------*
 |      name                 |      size
 |----------------------------------------------------------|
 |NVRAM_DOWNLOAD             |    0x0
 |----------------------------------------------------------|
 |NVRAM_CALIBRATION          |    NVRAM_CALIBRATION_SIZE
 |----------------------------------------------------------|
 |NVRAM_NORMAL               |    NVRAM_NORMAL_SIZE
 |----------------------------------------------------------|
 |TOTAL                      |
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#ifdef CONFIG_SUBSYS_AP
#define NVRAM_DIRECTORY_SIZE        0
#else
#define NVRAM_DIRECTORY_SIZE        NVRAM_ID_MAX * NVRAM_DIRECTORY_ITEM_SIZE
#endif

#ifndef NVRAM_DIFF_SIZE
#define NVRAM_DIFF_SIZE                   0x2000
#endif

#ifndef NVRAM_DIRECTORY_DEVICE_NAME
#define NVRAM_DIRECTORY_DEVICE_NAME          "nvdir"
#endif

#ifndef NVRAM_DOWNLOAD_DEVICE_NAME
#define NVRAM_DOWNLOAD_DEVICE_NAME           "nv"
#endif

#ifndef NVRAM_CUSTOM_DOWNLOAD_DEVICE_NAME
#define NVRAM_CUSTOM_DOWNLOAD_DEVICE_NAME    "nvcust"
#endif

#ifndef NVRAM_NORMAL_DEVICE_NAME
#define NVRAM_NORMAL_DEVICE_NAME             "nvram_normal"
#endif

#ifndef NVRAM_CALIBRATION_DEVICE_NAME
#define NVRAM_CALIBRATION_DEVICE_NAME        "nvram_calibration"
#endif

#ifndef NVRAM_DIFF_DEVICE_NAME
#define NVRAM_DIFF_DEVICE_NAME               "nvdiff"
#endif

#if defined(FLASH0_NVRAM_BASE)
#define FLASH_NV_BASE       FLASH0_NVRAM_BASE
#define FLASH_NV_SIZE       FLASH0_NVRAM_SIZE
#elif defined(FLASH0_NV_BASE)
#define FLASH_NV_BASE       FLASH0_NV_BASE
#define FLASH_NV_SIZE       FLASH0_NV_SIZE
#endif

#if defined(FLASH0_NVCUST_BASE)
#define FLASH_NVCUST_BASE   FLASH0_NVCUST_BASE
#define FLASH_NVCUST_SIZE   FLASH0_NVCUST_SIZE
#else
#define FLASH_NVCUST_BASE   0x0
#define FLASH_NVCUST_SIZE   0x0
#endif

#if defined(FLASH0_NVRAM_DIFF_BASE)
#define FLASH_NVDIFF_BASE   FLASH0_NVRAM_DIFF_BASE
#define FLASH_NVDIFF_SIZE   FLASH0_NVRAM_DIFF_SIZE
#elif defined(FLASH0_NVDIFF_BASE)
#define FLASH_NVDIFF_BASE   FLASH0_NVDIFF_BASE
#define FLASH_NVDIFF_SIZE   FLASH0_NVDIFF_SIZE
#else
#define FLASH_NVDIFF_BASE   0
#define FLASH_NVDIFF_SIZE   0
#endif

/**---------------------------------------------------------------------------*
 ** Static Variables
 **---------------------------------------------------------------------------*/
#ifdef CONFIG_SUBSYS_CP
u8 __attribute__((aligned(32))) s_nvram_directory[NVRAM_DIRECTORY_SIZE] = {0};
#endif

#ifdef WIN32
#define FLASH_NV_BASE       0
#define FLASH_NV_SIZE       0
#define SHM_CP2AP_BASE      0
#define SHM_CP2AP_SIZE      0
#endif

#ifdef CONFIG_SUBSYS_AP
/*
 * nv download buffer multiplexed with the cp2ap share memory buffer.
 */
#if (SHM_CP2AP_SIZE < (FLASH_NV_SIZE + FLASH_NVCUST_SIZE + FLASH_NVDIFF_SIZE))
#error SHM_CP2AP_SIZE must >= (FLASH_NV_SIZE + FLASH_NVCUST_SIZE + FLASH_NVDIFF_SIZE)
#endif
#endif

static NVRAM_DEVICE_CONFIG nvram_device_config_table[NVRAM_DEVICE_MAXNUM] =
{
    {
        NVRAM_DIRECTORY_DEVICE_ID,
        NVRAM_DIRECTORY_DEVICE_NAME,
        0,
        0,
        0,
        0,
    },
    {
        NVRAM_DIFF_DEVICE_ID,
        NVRAM_DIFF_DEVICE_NAME,
        0,
        0,
        0,
        0,
    },
    {
        NVRAM_DOWNLOAD_DEVICE_ID,
        NVRAM_DOWNLOAD_DEVICE_NAME,
        0,
        0,
        0,
        0
    },
    {
        NVRAM_CUSTOM_DOWNLOAD_DEVICE_ID,
        NVRAM_CUSTOM_DOWNLOAD_DEVICE_NAME,
        0,
        0,
        0,
        0
    },
    {
        NVRAM_CALIBRATION_DEVICE_ID,
        NVRAM_CALIBRATION_DEVICE_NAME,
        0,
        0,
        0,
        0
    },
    {
        NVRAM_NORMAL_DEVICE_ID,
        NVRAM_NORMAL_DEVICE_NAME,
        0,
        0,
        0,
        0
    }
};

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/

#if defined(CONFIG_SUBSYS_CP) || defined(WIN32)
#ifndef WIN32
/**
* nvram_parse_cmdline - get config from cmdline.
*
* Parameter:
*   @key: nvram cmdline key.
*   @cfg: nvram config.
*
* Return:
*
*/
void nvram_parse_cmdline(cl_key_t key, NVRAM_DEVICE_CONFIG *cfg)
{
    char *cmd_str;
    u8 i, cnt;
    u32 addr;

    cmd_str = aic_cmdline_get_value(key);
    AIC_ASSERT(cmd_str);

    /* get ro addr */
    addr = strtoul(cmd_str, NULL, 16);
    cfg->ro_addr = addr;

    i = 0;
    cnt = 0;
    while (cmd_str[i] != '\0') {
        if (cmd_str[i] == ',') {
            if (cnt == 0) {
                /* get size */
                cfg->size = strtoul(cmd_str + i + 1, NULL, 16);
            } else if (cnt == 1) {
                cfg->addr = strtoul(cmd_str + i + 1, NULL, 16);
                break;
            }
            cnt++;
        }
        i++;
    }
}

/**
* nvram_cfg_init - nvram config init.
*
* Parameter:
*
* Return:
*
*/
void nvram_cfg_init(void)
{
    u8 idx;

    idx = NVRAM_DIRECTORY_DEVICE_ID;
    nvram_device_config_table[idx].size = NVRAM_DIRECTORY_SIZE;
    nvram_device_config_table[idx].addr = (u32)s_nvram_directory;

    idx = NVRAM_DIFF_DEVICE_ID;
    nvram_parse_cmdline(cl_key_nvdiff_info, &nvram_device_config_table[idx]);

    idx = NVRAM_DOWNLOAD_DEVICE_ID;
    nvram_parse_cmdline(cl_key_nv_info, &nvram_device_config_table[idx]);

    idx = NVRAM_CUSTOM_DOWNLOAD_DEVICE_ID;
    nvram_parse_cmdline(cl_key_nvcust_info, &nvram_device_config_table[idx]);

    idx = NVRAM_CALIBRATION_DEVICE_ID;
    nvram_parse_cmdline(cl_key_nvcali_info, &nvram_device_config_table[idx]);

    idx = NVRAM_NORMAL_DEVICE_ID;
    nvram_parse_cmdline(cl_key_nvnorm_info, &nvram_device_config_table[idx]);
}
#endif

/**
* aic_nvram_get_whitelist_cfg - get nvram whitelist id config.
*
* Parameter:
*   @whitelist: nvram whitelist item id.
*   @count: whitelist id count.
*
* Return:
*
*/
void aic_nvram_get_whitelist_cfg(u16 **whitelist, u32 *count)
{
    *count = sizeof(nvram_whitelist_id) / sizeof(u16);
    *whitelist = nvram_whitelist_id;
}

/**
* aic_nvram_get_max_id - get nvram max id.
*
* Parameter:
*
* Return:
*   return nvram max id.
*/
u16 aic_nvram_get_max_id()
{
    return NVRAM_ID_MAX;
}

/**
* aic_nvram_get_min_id - get nvram min id.
*
* Parameter:
*
* Return:
*   return nvram min id.
*/
u16 aic_nvram_get_min_id()
{
    return NVRAM_ID_MIN;
}
#else
/**
* nvram_cfg_init - nvram config init.
*
* Parameter:
*
* Return:
*
*/
void nvram_cfg_init()
{
    RG_DATA_T nvram_region;
    u8 idx, cmd_value[30];
    u32 addr, size;
    int ret;

    idx = NVRAM_DIFF_DEVICE_ID;
    nvram_device_config_table[idx].ro_addr = FLASH_NVDIFF_BASE;
    nvram_device_config_table[idx].size = FLASH_NVDIFF_SIZE;
    nvram_device_config_table[idx].addr = SHM_CP2AP_BASE;
    snprintf(cmd_value, sizeof(cmd_value), "%x,%x,%x",
            FLASH_NVDIFF_BASE, FLASH_NVDIFF_SIZE, SHM_CP2AP_BASE);
    aic_cmdline_update_key(cl_key_nvdiff_info, cmd_value);

    idx = NVRAM_DOWNLOAD_DEVICE_ID;
    nvram_device_config_table[idx].ro_addr = FLASH_NV_BASE;
    nvram_device_config_table[idx].size = FLASH_NV_SIZE;
    nvram_device_config_table[idx].addr = SHM_CP2AP_BASE + FLASH_NVDIFF_SIZE;
    snprintf(cmd_value, sizeof(cmd_value), "%x,%x,%x",
            FLASH_NV_BASE, FLASH_NV_SIZE, SHM_CP2AP_BASE + FLASH_NVDIFF_SIZE);
    aic_cmdline_update_key(cl_key_nv_info, cmd_value);

    idx = NVRAM_CUSTOM_DOWNLOAD_DEVICE_ID;
    nvram_device_config_table[idx].ro_addr = FLASH_NVCUST_BASE;
    nvram_device_config_table[idx].size = FLASH_NVCUST_SIZE;
    if (FLASH_NVCUST_SIZE) {
        nvram_device_config_table[idx].addr = SHM_CP2AP_BASE + FLASH_NV_SIZE + FLASH_NVDIFF_SIZE;
    } else {
        nvram_device_config_table[idx].addr = 0;
    }
    snprintf(cmd_value, sizeof(cmd_value), "%x,%x,%x",
            FLASH_NVCUST_BASE, FLASH_NVCUST_SIZE, SHM_CP2AP_BASE + FLASH_NV_SIZE + FLASH_NVDIFF_SIZE);
    aic_cmdline_update_key(cl_key_nvcust_info, cmd_value);

    ret = sys_get_psram_region(PSRAM_SHM_NV_RG, &nvram_region);
    AIC_ASSERT(ret == AIC_SUCCESS);

    addr = nvram_region.base;
    AIC_ASSERT(addr);

    size = NVRAM_CALIBRATION_SIZE + NVRAM_NORMAL_SIZE;
    AIC_ASSERT(nvram_region.size >= size);

    idx = NVRAM_CALIBRATION_DEVICE_ID;
    nvram_device_config_table[idx].ro_addr = 0x0;
    nvram_device_config_table[idx].size = NVRAM_CALIBRATION_SIZE;
    nvram_device_config_table[idx].addr = addr;
     snprintf(cmd_value, sizeof(cmd_value), "%x,%x,%x",
            0x0, NVRAM_CALIBRATION_SIZE, addr);
    aic_cmdline_update_key(cl_key_nvcali_info, cmd_value);

    addr += NVRAM_CALIBRATION_SIZE;

    idx = NVRAM_NORMAL_DEVICE_ID;
    nvram_device_config_table[idx].ro_addr = 0x0;
    nvram_device_config_table[idx].size = NVRAM_NORMAL_SIZE;
    nvram_device_config_table[idx].addr = addr;
    snprintf(cmd_value, sizeof(cmd_value), "%x,%x,%x",
            0x0, NVRAM_NORMAL_SIZE, addr);
    aic_cmdline_update_key(cl_key_nvnorm_info, cmd_value);
}
#endif

/**
* nvram_ap_device_config_table_get - get ap device config table.
*
* Return:
*    @device config table.
*/
NVRAM_DEVICE_CONFIG *nvram_device_config_table_get()
{
#ifdef WIN32
    u8 *ptr, idx;

    idx = NVRAM_DIRECTORY_DEVICE_ID;
    if (!nvram_device_config_table[idx].addr) {
        ptr = aos_zalloc(NVRAM_DIRECTORY_SIZE);
        nvram_device_config_table[idx].addr = (u32)ptr;
        nvram_device_config_table[idx].size = NVRAM_DIRECTORY_SIZE;
    }
    ++idx;

    if (!nvram_device_config_table[idx].addr) {
        ptr = aos_zalloc(NVRAM_DIFF_SIZE);
        AIC_ASSERT(ptr);
        nvram_device_config_table[idx].ro_addr = (u32)ptr;
        nvram_device_config_table[idx].addr = (u32)ptr;
        nvram_device_config_table[idx].size = NVRAM_DIFF_SIZE;
    }
    ++idx;

    if (!nvram_device_config_table[idx].addr) {
        ptr = aos_zalloc(NVRAM_DOWNLOAD_SIZE);
        AIC_ASSERT(ptr);
        nvram_device_config_table[idx].ro_addr = (u32)ptr;
        nvram_device_config_table[idx].addr = (u32)ptr;
        nvram_device_config_table[idx].size = NVRAM_DOWNLOAD_SIZE;
    }
    ++idx;

    if (!nvram_device_config_table[idx].addr) {
        ptr = aos_zalloc(NVRAM_CUSTOM_DOWNLOAD_SIZE);
        AIC_ASSERT(ptr);
        nvram_device_config_table[idx].ro_addr = (u32)ptr;
        nvram_device_config_table[idx].addr = (u32)ptr;
        nvram_device_config_table[idx].size = NVRAM_CUSTOM_DOWNLOAD_SIZE;
    }
    ++idx;

    if (!nvram_device_config_table[idx].addr) {
        ptr = aos_zalloc(NVRAM_CALIBRATION_SIZE);
        AIC_ASSERT(ptr);
        nvram_device_config_table[idx].ro_addr = (u32)ptr;
        nvram_device_config_table[idx].addr = (u32)ptr;
        nvram_device_config_table[idx].size = NVRAM_CALIBRATION_SIZE;
    }
    ++idx;

    if (!nvram_device_config_table[idx].addr) {
        ptr = aos_zalloc(NVRAM_NORMAL_SIZE);
        AIC_ASSERT(ptr);
        nvram_device_config_table[idx].addr = (u32)ptr;
        nvram_device_config_table[idx].size = NVRAM_NORMAL_SIZE;
    }
    ++idx;
#else
    static boolean cfg_flag = false;

    if (!cfg_flag) {
        nvram_cfg_init();
        cfg_flag = true;
    }
#endif

    return nvram_device_config_table;
}

/**
* aic_nvram_get_blk_cfg - get nvram block config.
*
* Parameter:
*   @blk_cfg: nvram block config.
*
* Return:
*
*/
void aic_nvram_get_blk_cfg(NVRAM_BLK_CFG *blk_cfg)
{
    blk_cfg->sector_size = NVRAM_DEVICE_SECTOR_SIZE;
    blk_cfg->blk_size = NVRAM_BLOCK_SIZE;
    blk_cfg->blk_chk_size = NVRAM_BLOCK_CHK_SIZE;
    blk_cfg->blk_data_size = NVRAM_BLOCK_DATA_SIZE;
#ifdef WIN32
    blk_cfg->flash_sector_size = NVRAM_FLASH_SECTOR_SIZE;
#endif
}

/**
* aic_nvram_get_devname - get nvram device name by category.
*
* Parameter:
*   @category: nvram category, NVRAM_CATEGORY.
*
* Return:
*   nvram device name.
*/
char *aic_nvram_get_devname(u8 category)
{
    char *name = NULL;

    if (category == NVRAM_CATEGORY_NORMAL) {
        name = (char *)NVRAM_NORMAL_DEVICE_NAME;
    } else if (category == NVRAM_CATEGORY_CALIBRATION) {
        name = (char *)NVRAM_CALIBRATION_DEVICE_NAME;
    } else {
        AIC_ASSERT(0);
    }

    return name;
}

/**
* aic_nvram_get_devname_bydevid - get nvram device name by device id.
*
* Parameter:
*   @devid: nvram device id, NVRAM_DEVICE_ID.
*
* Return:
*   nvram device name.
*/
char *aic_nvram_get_devname_bydevid(u32 devid)
{
    if (devid >= NVRAM_MAX_DEVICE_ID) {
        AIC_ASSERT(0);
        return NULL;
    }

    return ((char *)(nvram_device_config_table[devid].name));
}

