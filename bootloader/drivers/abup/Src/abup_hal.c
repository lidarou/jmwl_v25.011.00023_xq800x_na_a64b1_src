/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_hal.c
* Description: 逻辑、AT处理
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#include "abup_hal.h"
#include "abup_hal_uart.h"
#define ABUP_MAX_TMP_LEN 0x1000
abup_char abup_hal_buf[ABUP_BUF_MAX_LEN];
volatile abup_uint32 abup_tick = 0;
#if defined(ABUP_BOOTLOADER)&&!defined(__CCRL__)
#if (ABUP_DEFAULT_SECTOR_SIZE > ABUP_MAX_TMP_LEN)
ABUP_ALIGN(8) abup_uint8 abup_hal_flash_tmp[ABUP_MAX_TMP_LEN];
#elif defined(ABUP_C51)
ABUP_ALIGN(8) abup_uint8 abup_hal_flash_tmp[0x200];
#else
ABUP_ALIGN(8) abup_uint8 abup_hal_flash_tmp[ABUP_DEFAULT_SECTOR_SIZE];
#endif
#else
abup_update_struct abup_hal_flash_tmp;
#endif
abup_update_struct* abup_hal_update = NULL;
#ifndef ABUP_C51_SLIM
abup_weak abup_char* abup_get_firmware_version(void)
{
    abup_debug_override_function();
    return (abup_char*)NULL;
}
#ifndef ABUP_NO_WEAK
abup_weak abup_uint32 Abup_GetTick(void)
{
    abup_debug_override_function();
    return abup_tick;
}
abup_weak abup_uint abup_timer_dec_offset(void)
{
    abup_debug_override_function();
    return 1;
}
abup_weak void Abup_IncTick(void)
{
    abup_debug_override_function();
}
abup_weak void Abup_Delay(abup_uint32 Delay)
{
    abup_uint32 tickstart = Abup_GetTick();
    abup_uint32 wait = Delay;

    /* Add a period to guaranty minimum wait */
    if (wait < ABUP_MAX_DELAY)
    {
        wait += abup_timer_dec_offset();
    }

    while((Abup_GetTick() - tickstart) < wait)
    {
    }
}
#endif
#endif
abup_char* abup_get_buf(void)
{
    return abup_hal_buf;
}
abup_uint abup_get_buf_len(void)
{
    return ABUP_BUF_MAX_LEN;
}
#ifndef ABUP_C51_SLIM
void abup_reset_buf(void)
{
    abup_memset(abup_hal_buf,0,ABUP_BUF_MAX_LEN);
}
abup_weak abup_uint abup_hal_external_flash_get_blocksize(void)
{
    abup_debug_override_function();
    return ABUP_DEFAULT_SECTOR_SIZE;
}
#endif
#if !defined(WIN32)
abup_weak abup_uint abup_hal_get_blocksize(void)
{
    abup_debug_override_function();
    return ABUP_DEFAULT_SECTOR_SIZE;
}
#endif
abup_weak abup_uint abup_hal_get_true_blocksize(abup_uint8 type)
{
    ABUP_UNUSED(type);
    abup_debug_override_function();
    return abup_hal_get_blocksize();
}
abup_weak abup_uint32 abup_hal_get_info_addr(void)
{
    return ABUP_UPDATE_ADDR + ABUP_UPDATE_SIZE - abup_hal_get_true_blocksize(ABUP_FILETYPE_PATCH);
}
abup_weak abup_uint32 abup_hal_get_backup_addr(void)
{
    return ABUP_UPDATE_ADDR + ABUP_UPDATE_SIZE - 2*abup_hal_get_true_blocksize(ABUP_FILETYPE_PATCH);
}
#if !defined(WIN32)
abup_weak abup_uint32 abup_hal_get_delta_size(void)
{
    return ABUP_UPDATE_SIZE - 2*abup_hal_get_true_blocksize(ABUP_FILETYPE_PATCH);
}
#endif
abup_weak abup_uint32 abup_hal_get_app_addr(void)
{
#ifdef ABUP_FLASH_APP_ADDR
    return ABUP_FLASH_APP_ADDR;
#else
    return ABUP_FLASH_BASE_ADDR + ABUP_BL_SIZE;
#endif
}
abup_uint32 abup_hal_get_app_backup_addr(void)
{
#ifdef ABUP_FLASH_APP_BK_ADDR
    return ABUP_FLASH_APP_BK_ADDR;
#else
    return 0;
#endif
}
abup_uint32 abup_hal_get_app_backup_size(void)
{
#ifdef ABUP_FLASH_APP_BK_SIZE
    return ABUP_FLASH_APP_BK_SIZE;
#else
    return 0;
#endif
}
abup_uint32 abup_hal_get_bl_addr(void)
{
#ifdef ABUP_FLASH_BL_ADDR
    return ABUP_FLASH_BL_ADDR;
#else
    return 0;
#endif
}
abup_uint32 abup_hal_get_bl_size(void)
{
#ifdef ABUP_BL_SIZE
    return ABUP_BL_SIZE;
#else
    return 0;
#endif
}
abup_uint32 abup_hal_get_bl_backup_addr(void)
{
#ifdef ABUP_FLASH_BL_BKUP_ADDR
    return ABUP_FLASH_BL_BKUP_ADDR;
#else
    return 0;
#endif
}
abup_uint32 abup_hal_get_bl_backup_size(void)
{
#ifdef ABUP_BL_BKUP_SIZE
    return ABUP_BL_BKUP_SIZE;
#else
    return 0;
#endif
}
abup_weak abup_uint32 abup_hal_get_delta_addr(void)
{
    return ABUP_UPDATE_ADDR;
}
abup_bool abup_hal_erase_sector(abup_uint8 type,abup_uint32 addr)
{
    abup_int abup_result = 0;
    abup_int blocksize = (abup_int)abup_hal_get_true_blocksize(type);
    if((addr%(abup_uint32)blocksize) == 0)
    {
        abup_result = abup_hal_flash_erase(type,addr,(abup_uint)blocksize);
    }
    return (abup_result < 0)?abup_false:abup_true;
}
#if defined(ABUP_WIFI_SSID_PWD)
abup_bool abup_hal_check_ssid_pwd(abup_char* ssid,abup_uint16 ssid_len,abup_char* pwd,abup_uint16 pwd_len)
{
    abup_update_struct* update = abup_hal_get_update();
    if(update)
    {
        if((ssid_len > 2)
                &&(ssid_len < abup_sizeof(update->wifi_ssid))
                &&(pwd_len > 4)
                &&(pwd_len < abup_sizeof(update->wifi_pwd))
                &&(!abup_str_is_same_char(ssid,ssid_len,0))
                &&(!abup_str_is_same_char(ssid,ssid_len,0xFF))
                &&(!abup_str_is_same_char(pwd,pwd_len,0))
                &&(!abup_str_is_same_char(pwd,pwd_len,0xFF)))
        {
            return abup_true;
        }
    }
    return abup_false;
}
#endif
abup_int abup_hal_get_key_version(void)
{
    return ABUP_KEY_VERSION;
}
#if !defined(ABUP_OPENFOTA)
abup_uint8 abup_hal_get_serverNO(void)
{
    abup_update_struct* update = abup_hal_get_update();
    if(update)
        return update->serverNO;
    else
        return (abup_uint8)-1;
}
abup_weak abup_uint8* abup_hal_get_mid(void)
{
#if defined(WIN32) || defined(LINUX)
    abup_extern abup_uint8* abup_get_uuid(void);
    return abup_get_uuid();
#else
    abup_update_struct* update = abup_hal_get_update();
    if(update)
        return update->mid;
    else
        return NULL;
#endif
}
#ifndef ABUP_C51_SLIM
abup_uint16 abup_hal_get_update_result(void)
{
    abup_update_struct* update = abup_hal_get_update();
    if(update)
        return update->update_result;
    else
        return 0;
}
void abup_hal_set_update_result(abup_uint16 result)
{
    abup_update_struct* update = abup_hal_get_update();
    if(update)
    {
        update->update_result = result;
        abup_hal_set_update(update);
    }
}
#endif
abup_update_struct* abup_hal_get_update(void)
{
    return (abup_update_struct*)abup_hal_update;
}
abup_bool abup_hal_get_inited(void)
{
    if(abup_hal_update == NULL)
    {
        return abup_false;
    }
    else
    {
        return abup_true;
    }
}
abup_update_struct* abup_hal_init_update(void)
{
    if(abup_hal_update == NULL)
    {
        abup_uint abup_hal_update_size = abup_sizeof(abup_hal_flash_tmp);
#if defined(ABUP_UB)
        abup_uint8* p = NULL;
        abup_uint8* p1 = NULL;
        abup_uint32 crc;
#endif
#if defined(ABUP_BOOTLOADER)&&!defined(__CCRL__)
        abup_hal_update = (abup_update_struct*)abup_hal_flash_tmp;
#else
        abup_hal_update = (abup_update_struct*)&abup_hal_flash_tmp;
#endif
        abup_memset(abup_hal_update,0,(abup_int)abup_hal_update_size);
#if defined(ABUP_UB)
        p = (abup_uint8*)&abup_hal_update->crc;
        p1 = (abup_uint8*)abup_hal_update;
#endif
        abup_hal_flash_read(ABUP_FILETYPE_PATCH_INFO,0,(abup_uint8*)abup_hal_update,abup_hal_update_size);
#if defined(ABUP_UB)
        crc = calc_crc((abup_uint8*)abup_hal_update,p - p1);
        if(AbupIsBigEndian())
        {
            crc = AbupSW32(crc);
        }
#endif
        if((abup_hal_update->inited != 0x8)
#if defined(ABUP_UB)
                ||(abup_hal_update->crc != crc)
#endif
          )
        {
            abup_hal_flash_erase(ABUP_FILETYPE_PATCH_INFO,0,abup_hal_update_size);
#if defined(ABUP_UB)
            abup_memset(abup_hal_update,0,abup_sizeof(abup_update_struct));
#else
            abup_memset(abup_hal_update,0,(abup_int)abup_hal_update_size);
#endif
            abup_hal_update->inited = 0x8;
//            abup_hal_update->serverNO = 0x2;//测试服务器
#if defined(ABUP_UB)
            crc = calc_crc((abup_uint8*)abup_hal_update,p - p1);
            if(AbupIsBigEndian())
            {
                abup_hal_update->crc = AbupSW32(crc);
            }
            else
            {
                abup_hal_update->crc = crc;
            }
#endif
            abup_hal_flash_write(ABUP_FILETYPE_PATCH_INFO,0,(abup_uint8*)abup_hal_update,abup_hal_update_size);
            abup_memset(abup_hal_update,0,(abup_int)abup_hal_update_size);
            abup_hal_flash_read(ABUP_FILETYPE_PATCH_INFO,0,(abup_uint8*)abup_hal_update,abup_hal_update_size);
        }
    }

#if defined(ABUP_BOOTLOADER)&&!defined(__CCRL__)
#if defined(ABUP_UB)||defined(ABUP_UA)
    if((abup_hal_update->ShareFunc[0] != (void*)abup_get_uuid_ext)
            ||(abup_hal_update->ShareFunc[1] != (void*)abup_get_bid)
            ||(abup_hal_update->ShareFunc[2] != (void*)abup_check_key)
      )
    {
        abup_hal_update->ShareFunc[0] = (void*)abup_get_uuid_ext;
        abup_hal_update->ShareFunc[1] = (void*)abup_get_bid;
        abup_hal_update->ShareFunc[2] = (void*)abup_check_key;
        abup_hal_set_update(abup_hal_update);
    }
#endif
#endif
#if defined(ABUP_BOOT0_ENABLE)&&(ABUP_BOOT0_ENABLE == 1)
#else
#if defined(ABUP_BOOTLOADER)&&!defined(__CCRL__)
#if defined(ABUP_UB)||defined(ABUP_UA)
    if(abup_hal_update->bootupdate != 0)
    {
        abup_hal_update->bootupdate = 0;
        abup_hal_set_update(abup_hal_update);
    }
#endif
#endif
#endif
    return (abup_update_struct*)abup_hal_update;
}
#ifndef ABUP_C51_SLIM
abup_update_struct* abup_hal_rst_fota(void)
{
    abup_uint abup_hal_update_size = abup_sizeof(abup_hal_flash_tmp);
#if defined(ABUP_FILE)
    abup_memset((void *)abup_hal_update,0,abup_hal_update_size);
    abup_hal_flash_write(ABUP_FILETYPE_PATCH_INFO,0,(abup_uint8*)abup_hal_update, (abup_uint)abup_hal_update_size);
#else
    abup_hal_flash_erase(ABUP_FILETYPE_PATCH_INFO,0,abup_hal_update_size);
#endif
    abup_hal_update = NULL;
    return abup_hal_init_update();
}
#endif
abup_bool abup_hal_set_update(abup_update_struct *abup_update)
{
    abup_int abup_hal_update_size = abup_sizeof(abup_hal_flash_tmp);
    if(abup_hal_erase_sector(ABUP_FILETYPE_PATCH_INFO,0))
    {
#if defined(ABUP_UB)
        abup_uint8* p = (abup_uint8*)&abup_update->crc;
        abup_uint8* p1 = (abup_uint8*)abup_update;
        abup_update->crc = calc_crc((abup_uint8*)abup_update,p - p1);
        if(AbupIsBigEndian())
        {
            abup_update->crc = AbupSW32(abup_update->crc);
        }
#endif
        if(abup_hal_flash_write(ABUP_FILETYPE_PATCH_INFO,0,(abup_uint8*)abup_update, (abup_uint)abup_hal_update_size) == abup_hal_update_size)
        {
            return abup_true;
        }
    }
    return abup_false;
}
#ifndef ABUP_C51_SLIM
abup_weak void AbupHalDeltaWrite(abup_bool inited,abup_bool succ)
{
    ABUP_UNUSED(inited);
    ABUP_UNUSED(succ);
    abup_debug_override_function();
}
abup_bool abup_hal_write_delta(abup_uint16 index,abup_uint8* buf,abup_uint16 len)
{
    if(abup_hal_erase_sector(ABUP_FILETYPE_PATCH,index *ABUP_DATA_SEQ_MAX_LEN))
    {
        if(abup_hal_flash_write(ABUP_FILETYPE_PATCH,index *ABUP_DATA_SEQ_MAX_LEN, buf, len) == (abup_int)len)
        {
            AbupHalDeltaWrite(abup_hal_get_inited(),abup_true);
            return abup_true;
        }
    }
    AbupHalDeltaWrite(abup_hal_get_inited(),abup_false);
    return abup_false;
}
#endif
#endif
#if defined(ABUP_RTTHREAD)&&defined(PKG_USING_FAL)
const struct fal_partition *abup_hal_get_device(abup_uint8 type)
{
    abup_char* device = NULL;
    if(type == ABUP_FILETYPE_APP)
    {
        device = ABUP_RTTHREAD_APP;
    }
    else if((type == ABUP_FILETYPE_BACKUP)
            ||(type == ABUP_FILETYPE_PATCH)
            ||(type == ABUP_FILETYPE_PATCH_INFO))
    {
        device = ABUP_RTTHREAD_FLASH;
    }
    if(device != NULL)
    {
        return fal_partition_find(device);
    }
    else
    {
        return NULL;
    }
}
abup_uint32 abup_hal_get_device_addr(abup_uint8 type,abup_uint32 addr)
{
    abup_uint32 address = 0;
    if(type == ABUP_FILETYPE_APP)
    {
        address = addr;
    }
    else if(type == ABUP_FILETYPE_BACKUP)
    {
        address = ABUP_UPDATE_SIZE - 2*abup_hal_get_true_blocksize(type) + addr;
    }
    else if(type == ABUP_FILETYPE_PATCH)
    {
        address = addr;
    }
    else if(type == ABUP_FILETYPE_PATCH_INFO)
    {
        address = ABUP_UPDATE_SIZE - abup_hal_get_true_blocksize(type) + addr;
    }
    return address;
}
#else
abup_uint32 abup_hal_get_addr(abup_uint8 type,abup_uint32 offset)
{
    abup_uint32 address = (abup_uint32)-1;
#ifdef ABUP_OPENFOTA
    if(type == ABUP_FILETYPE_APP)
    {
        address = abup_hal_get_app_addr() + offset;
    }
#else
#if defined(ABUP_BOOT0_ENABLE)&&(ABUP_BOOT0_ENABLE == 1)
#if defined(ABUP_FLASH_BL_BKUP_ADDR)&&defined(ABUP_BL_BKUP_SIZE)&&(ABUP_BL_BKUP_SIZE == ABUP_BL_SIZE)
#ifdef ABUP_FLASH_BL_ADDR
    if(type == ABUP_FILETYPE_BL)
    {
        address = abup_hal_get_bl_addr() + offset;
    }
#endif
#endif
#ifdef ABUP_FLASH_BL_BKUP_ADDR
    if(type == ABUP_FILETYPE_BL_BACKUP)
    {
        address = abup_hal_get_bl_backup_addr() + offset;
    }
#endif
#else
    if(type == ABUP_FILETYPE_APP)
    {
        address = abup_hal_get_app_addr() + offset;
    }
    else if(type == ABUP_FILETYPE_BACKUP)
    {
        address = abup_hal_get_backup_addr() + offset;
    }
    else if(type == ABUP_FILETYPE_PATCH)
    {
        address = abup_hal_get_delta_addr() + offset;
    }
#ifdef ABUP_FLASH_APP_BK_ADDR
    else if(type == ABUP_FILETYPE_APP_BACKUP)
    {
        address = abup_hal_get_app_backup_addr() + offset;
    }
#endif
#endif
    else if(type == ABUP_FILETYPE_PATCH_INFO)
    {
        address = abup_hal_get_info_addr() + offset;
    }
#endif
    return address;
}

#if !defined(WIN32)
abup_weak abup_int AbupHalFlashRead(abup_uint8 type,abup_uint32 offset, abup_uint8 *buf, abup_uint bufsize)
{
    abup_debug_override_function();
    return abup_flash_read(abup_hal_get_addr(type,offset),buf,bufsize);
}
abup_weak abup_int AbupHalFlashWrite(abup_uint8 type,abup_uint32 offset, abup_uint8 *buf, abup_uint bufsize)
{
    abup_debug_override_function();
    return abup_flash_write(abup_hal_get_addr(type,offset),buf,bufsize);
}
abup_weak abup_int AbupHalFlashErase(abup_uint8 type,abup_uint32 offset,abup_uint bufsize)
{
    abup_debug_override_function();
    return abup_erase_sector(abup_hal_get_addr(type,offset),bufsize);
}
#endif
#endif
abup_weak void AbupHalFlashOperation(abup_bool inited,abup_uint8 type,abup_uint32 addr,abup_uint bufsize,abup_int result,ABUP_FLASH_OPERATION_TYPE optype)
{
    ABUP_UNUSED(inited);
    ABUP_UNUSED(type);
    ABUP_UNUSED(addr);
    ABUP_UNUSED(bufsize);
    ABUP_UNUSED(result);
    ABUP_UNUSED(optype);
    abup_debug_override_function();
}
#ifndef ABUP_NO_WEAK
abup_weak abup_int abup_hal_flash_read(abup_uint8 type,abup_uint32 addr, abup_uint8 *buf, abup_uint bufsize)
{
    abup_int result = 0;
#if defined(ABUP_RTTHREAD)&&defined(PKG_USING_FAL)
    result = fal_partition_read(abup_hal_get_device(type), abup_hal_get_device_addr(type,addr), buf, bufsize);
#else
    result = AbupHalFlashRead(type,addr,buf,bufsize);
#endif
#if defined(ABUP_AUTO_TEST_FOTA)
    AbupHalFlashOperation(abup_hal_get_inited(),type,addr,bufsize,result,ABUP_FLASH_OPERATION_READ);
#endif
    return result;
}

abup_weak abup_int abup_hal_flash_write(abup_uint8 type,abup_uint32 addr, abup_uint8 *buf, abup_uint bufsize)
{
    abup_int result = 0;
#if defined(ABUP_RTTHREAD)&&defined(PKG_USING_FAL)
    result = fal_partition_write(abup_hal_get_device(type), abup_hal_get_device_addr(type,addr), buf, bufsize);
#else
    result = AbupHalFlashWrite(type,addr,buf,bufsize);
#endif
#if defined(ABUP_AUTO_TEST_FOTA)
    AbupHalFlashOperation(abup_hal_get_inited(),type,addr,bufsize,result,ABUP_FLASH_OPERATION_WRITE);
#endif
    return result;
}
abup_weak abup_int abup_hal_flash_erase(abup_uint8 type,abup_uint32 addr, abup_uint bufsize)
{
    abup_int result = 0;
#if defined(ABUP_RTTHREAD)&&defined(PKG_USING_FAL)
    result = fal_partition_erase(abup_hal_get_device(type), abup_hal_get_device_addr(type,addr), bufsize);
#else
    result = AbupHalFlashErase(type,addr,bufsize);
#endif
#if defined(ABUP_AUTO_TEST_FOTA)
    AbupHalFlashOperation(abup_hal_get_inited(),type,addr,bufsize,result,ABUP_FLASH_OPERATION_ERASE);
#endif
    return result;
}
#endif
