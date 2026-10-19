/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_hal_flash.c
* Description: flash读写
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef ABUP_HAL_FLASH_C
#define ABUP_HAL_FLASH_C
#include "abup_hal_flash.h"
#include "abup_stdlib.h"
#include "console/console.h"
#include "fmd.h"
#include "abup_hal.h"


static char partition_name[MAX_PARTITION_NAME_SIZE] = {0};


abup_bool abup_flash_partition_name(abup_uint32 base_addr, abup_uint32 *addr)
{
    abup_bool ret = abup_true;
#if (LOG_DETAIL == 1)
    log_print("\nbase_addr:0x%x, real_offset:0x%x ", base_addr, *addr-base_addr);
#endif
    abup_memset(partition_name, 0, abup_sizeof(partition_name));
    switch (base_addr)
    {
        case FLASH0_BL_BASE:
            abup_strncpy(partition_name, "bl", strlen("bl"));
            break;
        case FLASH0_NVRAM_BASE:
            abup_strncpy(partition_name, "nvram", strlen("nvram"));
            break;
        case FLASH0_CP_BASE:
            abup_strncpy(partition_name, "cp", strlen("cp"));
            break;
        case FLASH0_CPRAM1_BASE:
        case 0x00000001:
            abup_strncpy(partition_name, "cpram1", strlen("cpram1"));
            break;
        case FLASH0_CPRAM2_BASE:
        case 0x00000002:
            abup_strncpy(partition_name, "cpram2", strlen("cpram2"));
            break;
        case FLASH0_ADSPDTCM_BASE:
        case 0x00000003:
            abup_strncpy(partition_name, "adspdtcm", strlen("adspdtcm"));
            break;
        case FLASH0_ADSPPTCM_BASE:
        case 0x00000004:
            abup_strncpy(partition_name, "adspptcm", strlen("adspptcm"));
            break;
        case FLASH0_ADSPRAM_BASE:
        case 0x00000005:
            abup_strncpy(partition_name, "adspram", strlen("adspram"));
            break;
        case FLASH0_MISC_BASE:
            abup_strncpy(partition_name, "misc", strlen("misc"));
            break;
        case FLASH0_AP_BASE:
            abup_strncpy(partition_name, "ap", strlen("ap"));
            break;
        case FLASH0_FOTA_BASE:
            abup_strncpy(partition_name, "fota", strlen("fota"));
            break;
        case FLASH0_FOTA_BAK_BASE:
            abup_strncpy(partition_name, "fota_bak", strlen("fota_bak"));
            break;
        default:
            if (base_addr >= FLASH0_FOTA_BASE && base_addr < FLASH0_FOTA_BAK_BASE)
            {
                base_addr = FLASH0_FOTA_BASE;
                abup_strncpy(partition_name, "fota", strlen("fota"));
            }
            else
            {
                abup_memset(partition_name, 0, abup_sizeof(partition_name));
                ret = abup_false;
            }
            break;
    }
    *addr -= base_addr;
    log_print("partition_name:[%s]\n", partition_name);

    return ret;
}

static abup_uint32 abup_get_addr(abup_uint8 type, abup_uint32 address)
{
    abup_uint32 base_addr = (abup_uint32)-1;

    if(type == ABUP_FILETYPE_APP)
        base_addr = abup_hal_get_app_addr();
    else if(type == ABUP_FILETYPE_BACKUP)
        base_addr = abup_hal_get_backup_addr();
    else if(type == ABUP_FILETYPE_PATCH)
        base_addr = abup_hal_get_delta_addr();
#ifdef ABUP_FLASH_APP_BK_ADDR
    else if(type == ABUP_FILETYPE_APP_BACKUP)
        base_addr = abup_hal_get_app_backup_addr();
#endif
    else if(type == ABUP_FILETYPE_PATCH_INFO)
        base_addr = abup_hal_get_info_addr();

    address += base_addr;
    if (abup_false == abup_flash_partition_name(base_addr, &address))
    {
        log_print("abup_get_addr error\n");
        return -1;
    }

    if (address > REG_BASE_MEM_NORFLASH)
        address -= base_addr;

    return address;
}

void abup_firmware_copy2flash(abup_uint32 des_addr, abup_uint8 *p_src, abup_int32 size)
{
    abup_uint32 block_num = size / ABUP_DEFAULT_SECTOR_SIZE;
    abup_uint32 left_size = size % ABUP_DEFAULT_SECTOR_SIZE;
    abup_uint32 offset = des_addr;
    abup_int ret = 0;
    abup_uint32 temp_base = abup_decompressfile_info.flashbase;
#if (LOG_DETAIL == 1)
    log_print("abup_firmware_copy2flash is running...\n");
#endif
    if (abup_false == abup_flash_partition_name(des_addr, &offset))
    {
        log_print("abup_flash_partition_name error\n");
        return;
    }
    abup_decompressfile_info.flashbase = 0;

    while(block_num)
    {
        ret = abup_erase_sector(offset, ABUP_DEFAULT_SECTOR_SIZE);
        if (ret != ABUP_DEFAULT_SECTOR_SIZE)
        {
            log_print("abup_erase_sector-1 error(%d)\r\n", ret);
        }

        ret = abup_flash_write(offset, p_src, ABUP_DEFAULT_SECTOR_SIZE);
        if (ret != ABUP_DEFAULT_SECTOR_SIZE)
        {
            log_print("abup_flash_write-1 error(%d)\r\n", ret);
        }

        offset += ABUP_DEFAULT_SECTOR_SIZE;
        p_src += ABUP_DEFAULT_SECTOR_SIZE;
        block_num--;
    }
    if (left_size)
    {
        ret = abup_erase_sector(offset, left_size);
        if (ret != left_size)
        {
            log_print("abup_erase_sector-2 error(%d)\r\n", ret);
        }

        ret = abup_flash_write(offset, p_src, left_size);
        if (ret != left_size)
        {
            log_print("abup_flash_write-2 error(%d)\r\n", ret);
        }
    }
    abup_decompressfile_info.flashbase = temp_base;
}

abup_int AbupHalFlashRead(abup_uint8 type,abup_uint32 offset, abup_uint8 *buf, abup_uint bufsize)
{
    offset = abup_get_addr(type, offset);
    if (-1 == offset)
        return -98;

    return abup_flash_read(offset,buf,bufsize);
}

abup_int AbupHalFlashWrite(abup_uint8 type,abup_uint32 offset, abup_uint8 *buf, abup_uint bufsize)
{
    offset = abup_get_addr(type, offset);
    if (-1 == offset)
        return -98;

    return abup_flash_write(offset,buf,bufsize);
}

abup_int AbupHalFlashErase(abup_uint8 type,abup_uint32 offset,abup_uint bufsize)
{
    offset = abup_get_addr(type, offset);
    if (-1 == offset)
        return -98;

    return abup_erase_sector(offset,bufsize);
}

abup_int abup_flash_read(abup_uint32 offset, abup_uint8 *buf, abup_uint size)
{
    abup_uint32 img_ptn_handle = 0;
    abup_int ret = 0;

    if(!buf || size==0)
    {
        return -99;
    }
#if (LOG_DETAIL == 1)
    log_print("abup_flash_read offset:0x%x, size:0x%x, flashbase:0x%x\n", offset, size, abup_decompressfile_info.flashbase);
#endif
    if (0 != abup_decompressfile_info.flashbase)
    {
        if (abup_strncmp("cpram1", partition_name, strlen("cpram1")) == 0
            || abup_strncmp("cpram2", partition_name, strlen("cpram2")) == 0
            || abup_strncmp("adspdtcm", partition_name, strlen("adspdtcm")) == 0
            || abup_strncmp("adspptcm", partition_name, strlen("adspptcm")) == 0
            || abup_strncmp("adspram", partition_name, strlen("adspram")) == 0)
        {
        #if (LOG_DETAIL == 1)
            log_print("read cpram data copy -- \n");
            log_print("copy_offset:0x%x, copy_size:0x%x, flashbase:0x%x\n", offset, size, abup_decompressfile_info.flashbase);
        #endif
            abup_memcpy(buf, abup_decompressfile_info.p_dec_data + offset - abup_decompressfile_info.flashbase, size);
            return size;
        }
    }

    ret = fmd_open(partition_name, (unsigned int *)&img_ptn_handle);
    if (ret < 0) {
        log_print("open partition failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_read(img_ptn_handle, offset, size, buf);
    if (ret < 0) {
        log_print("read partition failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_close(img_ptn_handle);
    if (ret < 0) {
        log_print("close partition failed, errno(%d)!\n", ret);
        return ret;
    }
    #if (LOG_DETAIL == 1)
    log_print("fmd ok\n");
    #endif

    return size;    //success,must return size equal to parameter "size"
}

#if defined(__GNUC__)
#elif defined(__ICCARM__)||defined(__ICCRX__)
#pragma location = "RAMCODE"
#elif defined(__CC_ARM)
#pragma arm section code = "RAMCODE"
#endif
abup_int abup_flash_write(abup_uint32 addr, abup_uint8 *buf, abup_uint size)
{
    abup_uint32 img_ptn_handle = 0;
    abup_int ret = 0;

    if(!buf || size==0)
    {
        return -99;
    }
#if (LOG_DETAIL == 1)
    log_print("abup_flash_write offset:0x%x, size:0x%x, flashbase:0x%x\n", addr, size, abup_decompressfile_info.flashbase);
#endif
    if (0 != abup_decompressfile_info.flashbase)
    {
        if (abup_strncmp("cpram1", partition_name, strlen("cpram1")) == 0
            || abup_strncmp("cpram2", partition_name, strlen("cpram2")) == 0
            || abup_strncmp("adspdtcm", partition_name, strlen("adspdtcm")) == 0
            || abup_strncmp("adspptcm", partition_name, strlen("adspptcm")) == 0
            || abup_strncmp("adspram", partition_name, strlen("adspram")) == 0)
        {
        #if (LOG_DETAIL == 1)
            log_print("write cpram data copy -- \n");
            log_print("copy_offset:0x%x, copy_size:0x%x, flashbase:0x%x\n", addr, size, abup_decompressfile_info.flashbase);
        #endif
            abup_memcpy(abup_decompressfile_info.p_dec_data + addr - abup_decompressfile_info.flashbase, buf, size);
            return size;
        }
    }

    ret = fmd_open(partition_name, (unsigned int *)&img_ptn_handle);
    if (ret < 0) {
        log_print("open partition failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_write(img_ptn_handle, addr, size, buf);
    if (ret < 0) {
        log_print("write partition failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_close(img_ptn_handle);
    if (ret < 0) {
        log_print("close partition failed, errno(%d)!\n", ret);
        return ret;
    }
    #if (LOG_DETAIL == 1)
    log_print("fmd ok\n");
    #endif

    return size;    //success,must return size equal to parameter "size"
}

abup_int abup_erase_sector(abup_uint32 addr, abup_uint size)
{
    abup_uint32 img_ptn_handle = 0;
    abup_uint action_size = size;
    abup_int ret = 0;
    abup_uint32 real_addr = 0;
    if((action_size == 0) || (action_size/ABUP_DEFAULT_SECTOR_SIZE == 0))
    {
        action_size = ABUP_DEFAULT_SECTOR_SIZE;
    }
    real_addr = (addr/ABUP_DEFAULT_SECTOR_SIZE) * ABUP_DEFAULT_SECTOR_SIZE;

    if (0 != abup_decompressfile_info.flashbase)
    {
        if (abup_strncmp("cpram1", partition_name, strlen("cpram1")) == 0
            || abup_strncmp("cpram2", partition_name, strlen("cpram2")) == 0
            || abup_strncmp("adspdtcm", partition_name, strlen("adspdtcm")) == 0
            || abup_strncmp("adspptcm", partition_name, strlen("adspptcm")) == 0
            || abup_strncmp("adspram", partition_name, strlen("adspram")) == 0)
        {
            return ret;
        }
    }
    #if(LOG_DETAIL == 1)
    log_print("abup_erase_sector offset:0x%x, real_addr:0x%x,size:0x%x\n", addr,real_addr, action_size);
    #endif
    ret = fmd_open(partition_name, &img_ptn_handle);
    if (ret < 0) {
        log_print("open partition failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_erase(img_ptn_handle, real_addr, action_size);//addr
    if (ret < 0) {
        log_print("erase partition failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_close(img_ptn_handle);
    if (ret < 0) {
        log_print("close partition failed, errno(%d)!\n", ret);
        return ret;
    }

    return size;    //success,must return size equal to parameter "size"
}

abup_uint32 abup_get_partition_module(abup_uint32 base_addr)
{
    abup_uint32 module = 0;
    switch (base_addr)
    {
        case FLASH0_BL_BASE:
            module = 0x01;
            break;
        case FLASH0_CPRAM1_BASE:
        case 0x00000001:
            module = 0x02;
            break;
        case FLASH0_CPRAM2_BASE:
        case 0x00000002:
            module = 0x03;
            break;
        case FLASH0_ADSPDTCM_BASE:
        case 0x00000003:
            module = 0x04;
            break;
        case FLASH0_ADSPPTCM_BASE:
        case 0x00000004:
            module = 0x05;
            break;
        case FLASH0_ADSPRAM_BASE:
        case 0x00000005:
            module = 0x06;
            break;
        default:
            break;
    }

    log_print("module = %d\n", module);
    return module;
}


/* Erase run data partition */
abup_int abup_erase_rundata(void)
{
    abup_int ret = 0;
    u32 bkphandle = 0, bkpsize;

    ret = fmd_open("rundata", &bkphandle);
    if (!ret) {
        fmd_get_partition_size(bkphandle, &bkpsize);
        fmd_erase(bkphandle, 0, bkpsize);
    }
    return ret;
}

#if defined(__GNUC__)
#elif defined(__ICCARM__)||defined(__ICCRX__)
#elif defined(__CC_ARM)
#pragma arm section
#endif
#endif
