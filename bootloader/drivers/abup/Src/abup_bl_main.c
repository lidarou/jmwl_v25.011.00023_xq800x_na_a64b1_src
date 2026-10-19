/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_bl_main.c
* Description: 还原。
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#include "abup_hal_uart.h"
#include "abup_bl_main.h"
#if defined(ABUP_FOTA_ALGORITHM)&&(ABUP_FOTA_ALGORITHM == 0)
#include "abup_patch.h"
#endif
#include "abup_draw.h"
#include "abup_stdlib.h"
#include "abup_hal.h"
#if !defined(ABUP_8BIT)&&!defined(ABUP_C51)
#include "abup_bl_flash.h"
#include "peer_type.h"
#endif
#if defined(ABUP_TLV_SERVER)
#include "abup_tlv.h"
#include "peer.h"
#include "crc.h"
#endif
#include "abup_hal_flash.h"
#include <stdlib.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <boot_mode.h>
#include "console/console.h"
#include "initcall.h"
#include "platform/boot.h"
#include "platform/platform.h"
#include "cache.h"
#include "hlzma.h"
#include "lzma.h"


#if defined(ABUP_FOTA_ALGORITHM)&&(ABUP_FOTA_ALGORITHM == 0)&&!defined(ABUP_8BIT)&&!defined(ABUP_C51)
#ifdef ABUP_WORKING_BUFFER_LEN
#define ABUP_WORKING_BUFFER_MAX MLMIN(0x10000,ABUP_DEFAULT_SECTOR_SIZE)
#if (ABUP_WORKING_BUFFER_LEN > ABUP_WORKING_BUFFER_MAX)
#if (ABUP_WORKING_BUFFER_LEN > ABUP_DEFAULT_SECTOR_SIZE)&&!(ABUP_DEFAULT_SECTOR_SIZE > ABUP_WORKING_BUFFER_MAX)
#undef ABUP_WORKING_BUFFER_LEN
#define ABUP_WORKING_BUFFER_LEN ABUP_DEFAULT_SECTOR_SIZE
#else
#undef ABUP_WORKING_BUFFER_LEN
#define ABUP_WORKING_BUFFER_LEN 0x10000
#endif
#endif
#endif
#endif

abup_char progress[128] = {0};
#if defined(ABUP_FILE)
abup_extern void abup_set_current_file_path(abup_char* current);
abup_extern void* abup_file_open_close(abup_uint8 type,abup_uint8 open);
#endif
abup_extern abup_uint8* abup_get_uuid(void);
#if (ABUP_FOTA_ALGORITHM != 0)
abup_extern ML_BOOL peer_isCheckOldbin(void);
#endif
abup_extern abup_int abup_fotapatch_procedure(void);
abup_extern abup_bool abup_IsPackgeFound(void);
abup_extern void abup_get_package_number(abup_uint8 *delta_num);
abup_extern abup_int32 abup_get_package_info(multi_head_info_patch *info,abup_uint8 curr_delta);
ABUP_ALIGN(8) abup_int8 abup_malloc_buf[1] = {0};

static abup_uint8 *abup_p_malloc_buf = NULL;
//static abup_int32 abup_bl_debug_switch = 0;
abup_uint8 abup_delta_num=0;
abup_uint8 abup_curr_delta=1;
multi_head_info_patch PackageInfo = {0};

abup_extern void abup_printf(abup_char *str,abup_int len);
abup_extern abup_int abup_check_old(void);
abup_extern abup_int fotapatch_procedure(void);
abup_extern abup_int frame_memory_finalize(void);
abup_extern abup_bool abup_init_mem_pool(abup_uint8* buffer, abup_uint32 len);
#if (ABUP_FOTA_ALGORITHM != 0)||defined(LINUX)||defined(WIN32)
#if defined(ABUP_MAX_DELTA_NUM)&&(ABUP_MAX_DELTA_NUM > 1)
abup_char app_path[ABUP_MAX_DELTA_NUM][128];
#endif
#endif
abup_bool abup_flash_partition_name(abup_uint32 base_addr, abup_uint32 *addr);
static void abup_bl_get_delta_newsize(abup_uint32 *p_newsize);
abup_uint8 abup_bl_get_defaut_value(void);


#if defined(ABUP_FILE)
void abup_file_open(void)
{
    abup_file_open_close(ABUP_FILETYPE_APP,1);
    abup_file_open_close(ABUP_FILETYPE_BACKUP,1);
    abup_file_open_close(ABUP_FILETYPE_PATCH,1);
    abup_file_open_close(ABUP_FILETYPE_PATCH_INFO,1);
}
void abup_file_close(void)
{
    abup_file_open_close(ABUP_FILETYPE_APP,0);
    abup_file_open_close(ABUP_FILETYPE_BACKUP,0);
    abup_file_open_close(ABUP_FILETYPE_PATCH,0);
    abup_file_open_close(ABUP_FILETYPE_PATCH_INFO,0);
}
#endif
abup_weak void abup_patch_progress_ext(abup_bool PrePatch,abup_uint16 total,abup_uint16 current)
{
    static abup_int curr = -1;
    static abup_int percent = -1;
    if(curr != current)
    {
        curr = current;
        if(percent == current*100/total)
        {
            return;
        }
#if defined(ABUP_TLV_SERVER)
        abup_send_percent(curr*100/total);
#endif
        abup_memset(progress,0,abup_sizeof(progress));
#if (ABUP_FOTA_ALGORITHM == 0)
        ABUP_UNUSED(PrePatch);
#else
        if(PrePatch)
        {
            abup_memcpy(progress,"PrePatch:",9);
        }
#endif
        percent = current*100/total;
        abup_itoa(percent,progress + abup_strlen(progress),(abup_int)abup_sizeof(progress) - abup_strlen(progress),10,abup_false);
        abup_memcpy(progress + abup_strlen(progress),"%",1);
        abup_memcpy(progress + abup_strlen(progress),"\r\n",2);
        abup_bl_main_printf(progress);
#if defined(ABUP_FOTA_DISPLAY)
        if(PrePatch)
        {
            abup_memcpy(progress,"PrePatch:",9);
            AbupDisplayString(2,"PrePatch:%d%%",percent);
        }
        else
        {
            AbupDisplayString(2,"Progress:%d%%",percent);
        }
#endif
    }
}
void abup_patch_progress(abup_uint16 total,abup_uint16 current)
{
    abup_patch_progress_ext(abup_false,total,current);
}
#if defined(ABUP_CHECK_PATCH_FILE)&&(ABUP_CHECK_PATCH_FILE == 1)
#include "abup_hmd5.h"
abup_bool AbupCheckPatchFile(abup_uint32 patchsize)
{
    abup_bool ret = abup_true;
    abup_uint32 idx = 0;
    abup_uint8* tmp = (abup_uint8*)abup_bl_get_working_buffer();
    abup_int segmaxsize = abup_bl_get_working_buffer_len();

    ABUP_MD5_CTX md5;
    abup_uint8 md5c[16];
    abup_uint8 md5out[16];

    AbupMD5Init(&md5);
    for (idx=0; idx + segmaxsize < patchsize - 16; )
    {
        abup_memset(tmp,0,segmaxsize);
        if(abup_hal_flash_read(ABUP_FILETYPE_PATCH,idx, (abup_uint8 *)tmp, segmaxsize) != segmaxsize)
        {
            ret = abup_false;
            break;
        }
        AbupMD5Update(&md5, tmp, segmaxsize);
        idx += segmaxsize;
    }
    if ((ret)&&(patchsize - 16 - idx > 0 && patchsize - 16 - idx<=segmaxsize))
    {
        abup_memset(tmp,0,segmaxsize);
        if(abup_hal_flash_read(ABUP_FILETYPE_PATCH,idx, (abup_uint8 *)tmp, patchsize - 16 - idx) != (abup_int)(patchsize - 16 - idx))
            ret = abup_false;
        else
            AbupMD5Update(&md5, tmp, patchsize - 16 - idx);
    }
    if(ret)
    {
        AbupMD5Final(&md5,(abup_uint8 *)md5out);
        abup_bl_debug_printf("md5out: %s\r\n", md5out);//debug
        if(abup_hal_flash_read(ABUP_FILETYPE_PATCH,patchsize - 16, (abup_uint8 *)md5c, 16) == 16)
        {
            if(abup_memcmp(md5out,md5c,16) != 0)
            {
                abup_bl_main_printf("md5 calc fail\r\n");
                ret = abup_false;
            }
            else
            {
                abup_bl_main_printf("md5 calc ok\r\n");
            }
        }
        else
        {
            ret = abup_false;
        }
    }
    return ret;
}
#endif
#if (ABUP_FOTA_ALGORITHM == 0)||defined(LINUX)||defined(WIN32)
void AbupLusunProcedure(void)
{
    abup_int16 status = ABUP_FUDIFFNET_ERROR_NONE;
    abup_update_struct* update = abup_hal_init_update();
    if(update == NULL)
    {
        abup_bl_main_printf("Failed to read parameter\r\n");
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"Failed to read parameter");
#endif
        return;
    }
    if(update->update_result != ABUP_DOWNLOAD_SUCCESS)
    {
        abup_bl_main_printf("NO PACKAGE\r\n");
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"NO PACKAGE");
#endif
        return;
    }
#if defined(ABUP_CHECK_PATCH_FILE)&&(ABUP_CHECK_PATCH_FILE == 1)
    if(AbupLusunCheckPatchFile() == abup_false)
    {
#ifdef ABUP_DEBUG_MODE
        abup_bl_debug_printf("Error PatchFile\r\n");
#endif
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"Error PatchFile");
#endif
        return;
    }
#endif

    if(status == ABUP_FUDIFFNET_ERROR_NONE)
    {
#if defined(ABUP_FILE)
        abup_file_open();
#endif
        status = abup_fotapatch_procedure();
#if defined(ABUP_FILE)
        abup_file_close();
#endif
    }

    if(status == ABUP_FUDIFFNET_ERROR_NONE)
    {
        abup_bl_main_printf("Update succ\r\n");
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"Update succ");
#endif
        abup_hal_upgrade_success(abup_true);

    }
    else
    {
        abup_bl_main_printf("Update fail\r\n");
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"Update fail");
#endif
        abup_hal_upgrade_success(abup_false);
    }

    if(status == ABUP_UPDATE_AUTH_FAIL)
    {
        update->update_result = status;
        abup_hal_set_update(update);
    }

//    abup_hal_flash_erase(PEER_FILETYPE_PATCH,0,abup_hal_get_delta_size());
}
#endif
#if !defined(ABUP_8BIT)&&!defined(ABUP_C51)
void peer_watchdog_restart(void) {
#if defined(WIN32) || defined(LINUX)
    //peer_logout("WacthDogRestart");
#else
    //WacthDogRestart();
    //abup_bl_debug_print(NULL,"WacthDogRestart");
#endif
}
void abup_stream_init_decoder(STREAM_INF_FUN s_stream_fun[2],abup_int* stream_type)
{
    ABUP_UNUSED(s_stream_fun);
    ABUP_UNUSED(stream_type);
#if(ABUP_FOTA_ALGORITHM&1)||defined(LINUX)||defined(WIN32)
    peer_logout("WOSUN_BZ_ENABLE");
    *stream_type += STREAM_TYPE_BZ;
    s_stream_fun[0].init_fun = stream_init_bz_decoder;
    s_stream_fun[0].read_fun = stream_bz_read;
    s_stream_fun[0].end_fun = stream_bz_end;
#endif
#if(ABUP_FOTA_ALGORITHM&2)||defined(LINUX)||defined(WIN32)
    peer_logout("WOSUN_LZMA_ENABLE");
    *stream_type += STREAM_TYPE_LZMA;
    s_stream_fun[1].init_fun = stream_init_lzma_decoder;
    s_stream_fun[1].read_fun = stream_lzma_read;
    s_stream_fun[1].end_fun = stream_lzma_end;
#endif
}
abup_bool abup_get_pre_patch_flag(void)
{
    return abup_true;
}

abup_bool abup_get_pre_check_flag(void)
{
    return abup_true;
}


abup_uint32 abup_bl_get_diff_param_size(void)
{
    return ABUP_DIFF_PARAM_SIZE;
}


//#define BL_FOTA_UPDATE_INFO_RESERVE_SIZE       (512)      /*512byts ( FOTA_Update_Info size = 4*4 + 1*32 + 4*4 = 64 Bytes ) */
/*
 * RAW write update info
 */


abup_uint32 abup_bl_get_app_base(void)
{
    return PackageInfo.multi_bin_address;
}

abup_uint32 abup_bl_get_delta_base(void)
{
    return PackageInfo.multi_bin_offset;
}

void WacthDogRestart()
{
}


void abup_wait_sec(abup_int32 sec)
{
    ABUP_UNUSED(sec);
}

abup_bool abup_IsPartPackge(void)
{
    abup_uint8 data[5];
    abup_memset(data,0,abup_sizeof(data));
    abup_bl_read_backup_region(data, abup_sizeof(data));
    //abup_bl_debug_print(LOG_DEBUG, "%c%c%c%c%c\r\n",data[0],data[1],data[2],data[3],data[4]);
    if(data[0] == 'A' && data[1] == 'D' && data[2] == 'U' && data[3] == 'P' && data[4] == 'S')
    {
        //abup_bl_debug_print(LOG_DEBUG, "abup_IsPartPackge: YES!\r\n");
        return abup_true;
    }
    //abup_bl_debug_print(LOG_DEBUG, "abup_IsPartPackge: NO!\r\n");
    return abup_false;
}

#ifdef ABUP_COMPRESS_SUPPORT

abup_int abup_dec_partition(abup_uint8 *dest, abup_uint32 src_addr, abup_uint32 srclen)
{
    abup_int ret = 0;
    //abup_uint8 *p_data = abup_decompressfile_info.p_enc_data;
#if 0 //(LOG_DETAIL == 1)
    log_print("p_enc_data:");
    for (int i = 0; i < 64; i++)
    {
        if (i % 16 == 0)
            log_print("\n");
        log_print("%x ", p_data[i]);
    }
#endif

    dcache_invcc_by_range((abup_uint32)dest, srclen);

    ret = hlzma_dec_polling(src_addr+abup_strlen(HEADER_STR), (abup_uint32)dest);

    //log_print("\nhlzma_dec_polling ret(%d)\n", ret);

    dcache_inv_by_range((abup_uint32)dest, srclen);

#if 0//(LOG_DETAIL == 1)
    p_data = dest;
    log_print("\np_dec_data:");
    for (int i = 0; i < 64; i++)
    {
        if (i % 16 == 0)
            log_print("\n");
        log_print("%x ", p_data[i]);
    }
    //abup_firmware_copy2flash(FLASH0_CP_BASE, abup_decompressfile_info.p_dec_data, ABUP_DEC_DATA_SIZE);
#endif

    return ret;
}

abup_decompress_file_t abup_decompressfile_info = {
                                                .p_dec_data = NULL,
                                                .p_enc_data = NULL,
                                                .dec_len = 0,
                                                .enc_len = 0,
                                                .flashbase = 0};

static abup_uint32 abup_partition_addr = 0;
static abup_bool decompress_done = abup_false;
static abup_bool update_ready = abup_false;


static void abup_firmware_copy2backup(abup_uint32 partition_addr, abup_uint32 size)
{
    abup_uint8 *p_enc = NULL;
    abup_uint32 offset = partition_addr;
    abup_uint32 temp_base = abup_decompressfile_info.flashbase;
    abup_update_struct* update = abup_hal_get_update();
    abup_uint32 cur_module = 0;

#if 0//(LOG_DETAIL == 1)
    log_print("abup_firmware_copy2backup is running...\n");
#endif
    if (NULL == abup_decompressfile_info.p_enc_data)
    {
        abup_decompressfile_info.p_enc_data = abup_bl_get_working_buffer();

        if (NULL == abup_decompressfile_info.p_enc_data)
        {
            //abup_bl_main_printf("== p_enc_data malloc error ==\r\n");
            return;
        }
    }
    else
        abup_bl_main_printf("abup_decompressfile_info.p_enc_data ready\n");

    p_enc = abup_decompressfile_info.p_enc_data;

    cur_module = abup_get_partition_module(partition_addr);
    log_print("update->default_module:%d,cur_module:%d\n", update->default_module,cur_module);

    if (update->default_module == 0x01) //BL already backup
    {
        if (partition_addr != FLASH0_BL_BASE)
        {
            goto backup_action;
        }
        else
            return; //prevent to rewrite error BL data in BAK partition,causes device can not restart boot
    }
    else if ((update->default_module == cur_module)&&(update->default_module != 0))    //compressd already backup
    {
        if (partition_addr != FLASH0_BL_BASE)
        {
            abup_hal_flash_read(ABUP_FILETYPE_APP_BACKUP, 0, p_enc, size);
            //copy from backup partition to compress firmware partition
            abup_firmware_copy2flash(partition_addr, p_enc, size);
            abup_bl_main_printf("compress firmware restore...\n");
        }
    }
    else
    {
    backup_action:
        if (abup_true == update_ready)
        {
            abup_decompressfile_info.flashbase = 0;
            if (abup_false == abup_flash_partition_name(partition_addr, &offset))
            {
                log_print("abup_flash_partition_name error\n");
                return;
            }
            abup_flash_read(offset, p_enc, size);
            abup_decompressfile_info.flashbase = temp_base;

            abup_firmware_copy2flash(ABUP_FLASH_APP_BK_ADDR, p_enc, size);

            if(partition_addr == FLASH0_BL_BASE)
                update->default_module = 0X01;  //BL backup data exist
            else
                update->default_module = cur_module;  //compressd backup data exist
            abup_hal_set_update(update);
        }
    }

    //abup_memset(abup_decompressfile_info.p_enc_data, 0, ABUP_ENC_DATA_SIZE);
}

abup_uint32 abup_flash_partition_handler(void *p_info)
{
    multi_head_info_patch *info = (multi_head_info_patch *)p_info;

    if (NULL != info)
    {
    #if 0//(LOG_DETAIL == 1)
        log_print( "abup_flash_partition_handler multi_bin_address:0x%x\n\r", info->multi_bin_address);
    #endif

        switch (info->multi_bin_address)
        {
            case 0x00000001:
            {
                abup_decompressfile_info.flashbase = 0x00000001;
                return FLASH0_CPRAM1_BASE;
            }
            case 0x00000002:
            {
                abup_decompressfile_info.flashbase = 0x00000002;
                return FLASH0_CPRAM2_BASE;
            }
            case 0x00000003:
            {
                abup_decompressfile_info.flashbase = 0x00000003;
                return FLASH0_ADSPDTCM_BASE;
            }
            case 0x00000004:
            {
                abup_decompressfile_info.flashbase = 0x00000004;
                return FLASH0_ADSPPTCM_BASE;
            }
            case 0x00000005:
            {
                abup_decompressfile_info.flashbase = 0x00000005;
                return FLASH0_ADSPRAM_BASE;
            }
            case FLASH0_BL_BASE:
            {
                abup_firmware_copy2backup(FLASH0_BL_BASE, FLASH0_BL_SIZE);
                return FLASH0_BL_BASE;
            }
            default:
                abup_decompressfile_info.flashbase = 0;
                break;
        }
    }

    return 0;
}

abup_uint32 abup_hal_get_app_addr(void)
{
    return abup_bl_get_app_base();
}

static void abup_bl_get_app_oldsize(abup_int32 *p_oldsize)
{
    PATCH_HEADER patchheader = {0};
    abup_int ret = 0;
#if 0 //(LOG_DETAIL == 1)
    log_print("abup_bl_get_app_oldsize is running\n");
#endif

    ret = abup_hal_flash_read(ABUP_FILETYPE_PATCH, abup_bl_get_delta_base(), (abup_uint8*)&patchheader, abup_sizeof(PATCH_HEADER));
    if (ret >= 0) {
    }
    else {
        log_print("error read !");
        return;
    }
    //log_print("Current delta file oldsize :%d\n", patchheader.oldsize);

    *p_oldsize = patchheader.oldsize;
}

static void abup_decompress_procedure(abup_uint32 addr)
{
    abup_int ret = -1;
    abup_int32 oldsize = 0;
    abup_uint32 newsize = 0;
    abup_uint32 enc_partition_len = 0;
#if (LOG_DETAIL == 1)
    log_print("abup_decompress_procedure addr:0x%x\n", addr);
#endif

    if (abup_true == decompress_done)
    {
        goto end;
    }
    switch(addr)
    {
        case FLASH0_CPRAM1_BASE:
        case FLASH0_CPRAM2_BASE:
        case FLASH0_ADSPDTCM_BASE:
        case FLASH0_ADSPPTCM_BASE:
        case FLASH0_ADSPRAM_BASE:
            {
                abup_bl_get_app_oldsize(&oldsize);
                abup_bl_get_delta_newsize(&newsize);
                if (0 != oldsize)
                {
                    abup_decompressfile_info.dec_len = oldsize;
                    if(newsize > oldsize)
                    {
                        abup_decompressfile_info.dec_len = newsize;
                        log_print("abup_decompress_procedure use newsize oldsize:%d,newsize:%d\n", oldsize,newsize);
                    }

                    if (addr == FLASH0_CPRAM1_BASE)
                        enc_partition_len = FLASH0_CPRAM1_SIZE;
                    else if (addr == FLASH0_CPRAM2_BASE)
                        enc_partition_len = FLASH0_CPRAM2_SIZE;
                    else if (addr == FLASH0_ADSPDTCM_BASE)
                        enc_partition_len = FLASH0_ADSPDTCM_SIZE;
                    else if (addr == FLASH0_ADSPPTCM_BASE)
                        enc_partition_len = FLASH0_ADSPPTCM_SIZE;
                    else if (addr == FLASH0_ADSPRAM_BASE)
                        enc_partition_len = FLASH0_ADSPRAM_SIZE;

                    abup_firmware_copy2backup(addr, enc_partition_len);

                    if (NULL == abup_decompressfile_info.p_dec_data)
                    {
                        //int len =(ABUP_DEC_DATA_SIZE + SIZE_4K) & ~(SIZE_4K- 1);
                        abup_decompressfile_info.p_dec_data = (abup_uint8 *)malloc(ABUP_DEC_DATA_SIZE/*len*/);

                        if (NULL == abup_decompressfile_info.p_dec_data)
                        {
                            abup_bl_main_printf("== p_dec_data malloc error ==\r\n");
                            return;
                        }
                    }
                    else
                        abup_bl_main_printf("abup_decompressfile_info.p_dec_data ready\r\n");

                    abup_memset(abup_decompressfile_info.p_dec_data, abup_bl_get_defaut_value(), ABUP_DEC_DATA_SIZE);
                    ret = abup_dec_partition(abup_decompressfile_info.p_dec_data, addr, abup_decompressfile_info.dec_len);
                    if(abup_decompressfile_info.dec_len > ABUP_DEC_DATA_SIZE)
                    {
                        abup_bl_main_printf("error dec_len > ABUP_DEC_DATA_SIZE!\r\n");
                        return;
                    }

                    if (AIC_SUCCESS != ret)
                        log_print("abup_dec_partition error(%d)\r\n", ret);

                    if (abup_true == update_ready)
                    {
                        decompress_done = abup_true;
                    }
                }
                else
                {
                    //log_print("abup_bl_get_app_oldsize error\r\n");
                }
            }
            break;
        default:
            break;
    }
end:
    log_print("abup_decompress_procedure exit\n");
}

static void abup_firmware_backup_erase(void)
{
    abup_int ret = 0;
    abup_update_struct* update = abup_hal_get_update();

    update->default_module = 0X00;  //backup default_module erased
    abup_hal_set_update(update);

    ret = abup_hal_flash_erase(ABUP_FILETYPE_APP_BACKUP, 0, ABUP_ENC_DATA_SIZE);
    if (ret != ABUP_ENC_DATA_SIZE)
    {
        log_print("abup_firmware_backup_erase error(%d)\r\n", ret);
    }

    #if (LOG_DETAIL == 1)
    log_print("abup_firmware_backup_erase ok\n");
    #endif
}

static void abup_bl_get_delta_newsize(abup_uint32 *p_newsize)
{
    abup_uint32 newsize = 0;
    abup_uint8 header[18] = {0};
    PATCH_HEADER patchheader = {0};

    abup_hal_flash_read(ABUP_FILETYPE_PATCH, abup_bl_get_delta_base(), (abup_uint8*)&patchheader, sizeof(PATCH_HEADER));
    abup_hal_flash_read(ABUP_FILETYPE_PATCH, abup_bl_get_delta_base() + sizeof(PATCH_HEADER) + (4*patchheader.blocknum)*2, header, 18);

    if (abup_memcmp(header, "wosun07", 7) != 0)
    {
        //log_print("read header error, no wosun07\r\n");
        return;
    }
    abup_memcpy(&newsize, header+10, 4);
    //log_print("Current delta file newsize:%d\n", newsize);

    *p_newsize = newsize;
}

static void abup_compress_procedure(abup_uint32 addr)
{
    abup_int ret = -1;
    abup_uint32 newsize = 0;
    abup_uint8 *p_data = abup_decompressfile_info.p_enc_data+abup_strlen(HEADER_STR);
    abup_uint8 *p_src = abup_decompressfile_info.p_dec_data;
   // abup_uint32 data_size = 0;

    abup_decompressfile_info.flashbase = 0;
    switch (addr)
    {
    case FLASH0_CPRAM1_BASE:
    case FLASH0_CPRAM2_BASE:
    case FLASH0_ADSPDTCM_BASE:
    case FLASH0_ADSPPTCM_BASE:
    case FLASH0_ADSPRAM_BASE:
        {
            if (abup_false == update_ready) //no backup data yet,break
            {
                break;
            }
            abup_bl_get_delta_newsize(&newsize);
            abup_decompressfile_info.dec_len = newsize;

            ret = enc_partition(p_data, (unsigned int *)&newsize, p_src, newsize);
            if (0 != ret)
            {
                log_print("enc_partition error(0x%x)\r\n", ret);
                //abup_firmware_copy2flash(FLASH0_CP_BASE, abup_decompressfile_info.p_dec_data, abup_decompressfile_info.dec_len);
                return;
            }
            else
            {
                //log_print("==> output compress dat asize:%d\n", newsize);
                abup_decompressfile_info.enc_len = newsize;
            }
            abup_memmove(abup_decompressfile_info.p_enc_data, HEADER_STR, abup_strlen(HEADER_STR));

            //abup_firmware_copy2flash(FLASH0_CP_BASE, abup_decompressfile_info.p_enc_data, newsize+abup_strlen(HEADER_STR)+13);
            abup_firmware_copy2flash(addr, abup_decompressfile_info.p_enc_data, abup_decompressfile_info.enc_len+abup_strlen(HEADER_STR)+13);

            abup_firmware_backup_erase();
            abup_memset(abup_decompressfile_info.p_dec_data, 0, ABUP_DEC_DATA_SIZE);
            abup_memset(abup_decompressfile_info.p_enc_data, 0, ABUP_DEC_DATA_SIZE);

            decompress_done = abup_false;
        }
        break;
    case FLASH0_BL_BASE:
        abup_firmware_backup_erase();
        break;
    default:
        break;
    }
}

static void abup_buffer_free(void)
{
    if (NULL != abup_decompressfile_info.p_enc_data)
    {
        free(abup_decompressfile_info.p_enc_data);
        abup_decompressfile_info.p_enc_data = NULL;
        abup_p_malloc_buf = NULL;
    }
    if (NULL != abup_decompressfile_info.p_dec_data)
    {
        free(abup_decompressfile_info.p_dec_data);
        abup_decompressfile_info.p_dec_data = NULL;
    }
    update_ready = abup_false;
}


void abup_printf(abup_char *data,abup_int len)
{
    ABUP_UNUSED(len);

    log_print(data);
}

abup_uint8* abup_bl_get_working_buffer(void)
{
    if (NULL == abup_p_malloc_buf)
    {
        abup_p_malloc_buf = (abup_uint8 *)malloc(abup_bl_get_working_buffer_len());
        if (NULL == abup_p_malloc_buf)
        {
            abup_bl_main_printf("== abup_bl_get_working_buffer malloc error ==\r\n");
            return NULL;
        }
    }

    return abup_p_malloc_buf;
}


#endif
void AbupWosunProcedure(void)
{
#if (ABUP_FOTA_ALGORITHM != 0)||defined(LINUX)||defined(WIN32)
    abup_int status = ABUP_FUDIFFNET_ERROR_NONE;
#if defined(ABUP_MAX_DELTA_NUM)&&(ABUP_MAX_DELTA_NUM > 1)
    abup_int all_status[ABUP_MAX_DELTA_NUM];
#endif
    abup_bl_debug_print(LOG_DEBUG, "AbupWosunProcedure  4.0\r\n");

    abup_update_struct* update = abup_hal_init_update();
    if(update == NULL)
    {
        abup_bl_main_printf("Failed to read parameter\r\n");
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"Failed to read parameter");
#endif
        return;
    }
     abup_bl_debug_print(LOG_DEBUG, "update->update_result=%d!\r\n",update->update_result);//debug
     //update->update_result = ABUP_DOWNLOAD_SUCCESS;//open for test only
    if((update->update_result != ABUP_DOWNLOAD_SUCCESS)||!abup_IsPackgeFound())
    {
        abup_bl_main_printf("NO PACKAGE\r\n");
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"NO PACKAGE");
#endif
        return;
    }
#if defined(ABUP_CHECK_PATCH_FILE)&&(ABUP_CHECK_PATCH_FILE == 1)
    if(AbupWosunCheckPatchFile() == abup_false)
    {
#ifdef ABUP_DEBUG_MODE
        abup_bl_debug_printf("Error PatchFile\r\n");
#endif
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"Error PatchFile");
#endif
        return;
    }
#endif



    /*Init memory buffer*/

#ifndef EXTERNAL_MEMORY_MANAGEMENT
    if (NULL == abup_bl_get_working_buffer())
    {
#ifdef ABUP_FOTA_ENABLE_DIFF_ERROR
        // abup_bl_debug_print(LOG_DEBUG, "Can not alloc UPI working buffer\n");
#endif
        status=ABUP_FUDIFFNET_ERROR_INSUFFICIENT_WORKING_BUF;
    }

//    peer_setPrePatch(TRUE);
    peer_setCheckOldbin(TRUE);
#endif
#if defined(ABUP_MAX_DELTA_NUM)&&(ABUP_MAX_DELTA_NUM > 1)
    abup_get_package_number(&abup_delta_num);

    if ((abup_delta_num > ABUP_MAX_DELTA_NUM) || (abup_delta_num == 0))
    {
        abup_bl_main_printf("abup_delta_num error! exit\r\n");
        return;
    }
#endif
#ifndef ABUP_PURE_ALGORITHM
    if(peer_isCheckOldbin())
    {
#if defined(ABUP_MAX_DELTA_NUM)&&(ABUP_MAX_DELTA_NUM > 1)
        while(abup_curr_delta <= abup_delta_num)
        {
            abup_memset(&PackageInfo,0,sizeof(multi_head_info_patch));
#if defined(ABUP_FILE)
            abup_set_current_file_path(app_path[abup_curr_delta -1] );
            abup_bl_main_printf(app_path[abup_curr_delta -1]  );
            abup_bl_main_printf("\r\n");
#endif
#endif
#if defined(ABUP_FILE)
            abup_file_open();
#endif
            status=abup_get_package_info(&PackageInfo,abup_curr_delta);
            if(status == ABUP_FUDIFFNET_ERROR_NONE)
            {
            #ifdef ABUP_COMPRESS_SUPPORT
                abup_partition_addr = abup_flash_partition_handler(&PackageInfo);
                if (0 != abup_partition_addr)
                {
                    abup_decompress_procedure(abup_partition_addr);
                }
            #endif

                status = abup_check_old();
            }
#if defined(ABUP_FILE)
            abup_file_close();
#endif
#if defined(ABUP_MAX_DELTA_NUM)&&(ABUP_MAX_DELTA_NUM > 1)
            abup_bl_debug_print(LOG_DEBUG, "check old abup_curr_delta=%d status=%d!\r\n",abup_curr_delta,status);//debug
            all_status[abup_curr_delta -1] = status;
            if((status == ABUP_FUDIFFNET_ERROR_NONE)||(status == RETCODE_FOTA_PROCESS_PATCHED))
            {
                abup_curr_delta++;
                continue;
            }
            else
            {
                break;
            }
        }
        if(status == ABUP_FUDIFFNET_ERROR_NONE)
        {
            abup_curr_delta = 1;
            update_ready = abup_true;
            abup_bl_main_printf("check old bin end here\r\n");
        }
#endif

    }
#endif
    if(status == ABUP_FUDIFFNET_ERROR_NONE)
    {
#if defined(ABUP_MAX_DELTA_NUM)&&(ABUP_MAX_DELTA_NUM > 1)
        while(abup_curr_delta <= abup_delta_num)
        {
            if(all_status[abup_curr_delta -1] == RETCODE_FOTA_PROCESS_PATCHED)
            {
                abup_curr_delta++;
                continue;
            }

            abup_memset(&PackageInfo,0,sizeof(multi_head_info_patch));
#if defined(ABUP_FILE)
            abup_set_current_file_path(app_path[abup_curr_delta -1] );
            abup_bl_main_printf(app_path[abup_curr_delta -1]  );
            abup_bl_main_printf("\r\n");
#endif
#endif
#if defined(ABUP_FILE)
            abup_file_open();
#endif
            status=abup_get_package_info(&PackageInfo,abup_curr_delta);
            if(status == ABUP_FUDIFFNET_ERROR_NONE)
            {
            #ifdef ABUP_COMPRESS_SUPPORT
                abup_partition_addr = abup_flash_partition_handler(&PackageInfo);
                if (0 != abup_partition_addr)
                {
                    abup_decompress_procedure(abup_partition_addr);
                }
            #endif

                status = fotapatch_procedure();
            }
#if defined(ABUP_FILE)
            abup_file_close();
#endif
#if defined(ABUP_MAX_DELTA_NUM)&&(ABUP_MAX_DELTA_NUM > 1)
            if(status == ABUP_FUDIFFNET_ERROR_NONE || (status == RETCODE_FOTA_PROCESS_PATCHED))
            {
            #ifdef ABUP_COMPRESS_SUPPORT
                abup_compress_procedure(abup_partition_addr);
            #endif

                abup_curr_delta++;
                continue;
            }
            else
            {
                break;
            }
        }
#endif
    }
    if((status == ABUP_FUDIFFNET_ERROR_NONE)||(status == RETCODE_FOTA_PROCESS_PATCHED))
    {
        /* Erase rundata if success */
        abup_erase_rundata();
        abup_bl_main_printf("Update succ\r\n");
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"Update succ");
#endif
        abup_hal_upgrade_success(abup_true);
        bl_reset();
        while(1);
    }
    else
    {
        abup_bl_main_printf("Update fail\r\n");
#if defined(ABUP_FOTA_DISPLAY)
        AbupDisplayString(1,"Update fail");
#endif
        abup_hal_upgrade_success(abup_false);
    }

    if(status == ABUP_UPDATE_AUTH_FAIL)
    {
        update->update_result = (abup_uint16)status;
        abup_hal_set_update(update);
    }
    abup_buffer_free();
//    abup_bl_erase_delata();
//    abup_bl_erase_backup_region();

#ifdef ABUP_TEST_POWEROFF
    //abup_dump_test_info();
#endif

#endif
}
#endif
abup_uint32 abup_bl_get_curr_write_address(void)
{
    if (PackageInfo.multi_bin_address == 0x00000001)
    {
        return 0x00000001;
    }
    else if(PackageInfo.multi_bin_address == 0x00000002)
    {
        return 0x00000002;
    }
    else if(PackageInfo.multi_bin_address == 0x00000003)
    {
        return 0x00000003;
    }
    else if(PackageInfo.multi_bin_address == 0x00000004)
    {
        return 0x00000004;
    }
    else if(PackageInfo.multi_bin_address == 0x00000005)
    {
        return 0x00000005;
    }
    else
        return 0;
}

#if !defined(WIN32)
/*abup_weak abup_uint8* abup_bl_get_working_buffer(void)
{
#if defined(ABUP_FOTA_ALGORITHM)&&(ABUP_FOTA_ALGORITHM != 0)&&defined(EXTERNAL_MEMORY_MANAGEMENT)
    return NULL;
#else
    return (abup_uint8 *)abup_malloc_buf;//(RAM_BASE+1024*500);//(abup_malloc_buf);
#endif
}*/

abup_weak abup_uint abup_bl_get_working_buffer_len(void)
{
    return ABUP_WORKING_BUFFER_LEN;
}
abup_weak abup_bool abup_bl_is_limit_len(void)
{
#if (ABUP_WORKING_BUFFER_LEN == ABUP_DEFAULT_SECTOR_SIZE)
    return abup_true;
#else
    if(abup_hal_get_blocksize() == abup_bl_get_working_buffer_len())
    {
        return abup_true;
    }
    else
    {
        return abup_false;
    }
#endif
}
#endif

abup_uint8 abup_bl_get_defaut_value(void)
{
#if defined(ABUP_FLASH_DEFAULT_VALUE)
    return ABUP_FLASH_DEFAULT_VALUE;
#else
    return 0x00;
#endif
}
#define TEST    0
#if (TEST==1)
#define FLASH_SIZE 0X10000
void flash_test(void)
{
    uint8_t *test_buf = NULL;
    test_buf = malloc(FLASH_SIZE);
    abup_int ret = 0;

    PackageInfo.multi_bin_address = FLASH0_FOTA_BASE;
    ret = abup_hal_flash_read(ABUP_FILETYPE_APP, FLASH0_FOTA_BASE, test_buf, FLASH_SIZE);
    //log_print("abup_hal_flash_read1 ret:0x%x\r\n", ret);
    //log_print("test_buf:\r\n");
    for (int i = 0; i < 100; i++)
    {
        log_print("%x ", test_buf[i]);
        if (i % 16 == 0)
            log_print("\r\n");
    }

    ret = abup_hal_flash_erase(ABUP_FILETYPE_APP_BACKUP, ABUP_FLASH_APP_BK_ADDR, FLASH_SIZE);
    //log_print("abup_hal_flash_erase ret:0x%x\r\n", ret);

    ret = abup_hal_flash_write(ABUP_FILETYPE_APP_BACKUP, ABUP_FLASH_APP_BK_ADDR, test_buf, FLASH_SIZE);
    //log_print("abup_hal_flash_write ret:0x%x\r\n", ret);

    abup_memset(test_buf, 0, FLASH_SIZE);
    ret = abup_hal_flash_read(ABUP_FILETYPE_APP, ABUP_FLASH_APP_BK_ADDR, test_buf, FLASH_SIZE);
    //log_print("abup_hal_flash_read2 ret:0x%x\r\n", ret);
    //log_print("test_buf:\r\n");
    for (int i = 0; i < 100; i++)
    {
        //log_print("%x ", test_buf[i]);
        if (i % 16 == 0)
            log_print("\r\n");
    }

    free(test_buf);
    //abup_bl_main_printf("\r\n********************\r\n flash_test end...\r\n********************\r\n");
}
#endif
int AbupProcedure(void)
{
    abup_bl_main_printf("\r\n********************\n ENTER BOOTLOADER...\n********************\nRevision:91427\r\n");
#if (ABUP_FOTA_ALGORITHM == 0)
    AbupLusunProcedure();
#else
#if (TEST==0)
    AbupWosunProcedure();
#else
    flash_test();
#endif
#endif
#if defined(ABUP_UB)
#if defined(ABUP_UB_BID)
    abup_bl_main_printf("BID:");
    abup_bl_main_printf(ABUP_UB_BID);
    abup_bl_main_printf("\r\n");
#endif
#endif
#if !defined(ABUP_NO_KEY)
    abup_bl_main_printf("UUID:");
    abup_bl_main_printf((abup_char*)abup_get_uuid());
    abup_bl_main_printf("\r\n");
#endif
    return AIC_SUCCESS;
}

int abup_fota_init(void)
{
    platform_boot_mode_enterpoint_register(AbupProcedure, FOTA_BOOT);
    platform.boot_mode = NORMAL_BOOT;

    return AIC_SUCCESS;
}

driver_init_late(abup_fota_init);


