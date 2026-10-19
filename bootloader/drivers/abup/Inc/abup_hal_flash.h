/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_hal_flash.h
* Description: 
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef ABUP_HAL_FLASH_H
#define ABUP_HAL_FLASH_H

#include "abup_typedef.h"


#ifdef ABUP_COMPRESS_SUPPORT
typedef struct
{
    abup_uint8 *p_dec_data;
    abup_uint8 *p_enc_data;
    abup_uint32 dec_len;
    abup_uint32 enc_len;
    abup_uint32 flashbase;
}abup_decompress_file_t;

typedef struct patch_header
{
    abup_char  reserve1[8];
    abup_char  reserve2[8];
    abup_uint32 oldsize;
    abup_uint32 reserve4;
    abup_uint32 reserve5;
    abup_uint32 reserve6;
    abup_uint16 reserve7;
    abup_uint16 reserve8;
    abup_uint8  reserve9;
    abup_uint8  reserve10;
    abup_uint16 blocknum;
    abup_uint32 reserve11;
    abup_uint32 reserve12;
    abup_uint32 reserve13;
    abup_uint32 reserve14;
    abup_uint8  reserve15;
    abup_uint8  reserve16;
    abup_uint8  appended_patch_filecrc_flag;
    abup_uint8  reserved_u8[1];
    abup_uint32 reserved[3];
}PATCH_HEADER;

extern abup_decompress_file_t abup_decompressfile_info;

#endif

abup_uint32 abup_flash_partition_handler(void *p_info);
void abup_firmware_copy2flash(abup_uint32 des_addr, abup_uint8 *p_src, abup_int32 size);
abup_bool abup_flash_partition_name(abup_uint32 base_addr, abup_uint32 *addr);


extern abup_int abup_erase_rundata(void);
extern abup_int abup_erase_sector(abup_uint32 addr, abup_uint size);
extern abup_int abup_flash_write(abup_uint32 addr, abup_uint8 *buf, abup_uint size);
extern abup_int abup_flash_read(abup_uint32 offset, abup_uint8 *buf, abup_uint size);
extern abup_uint32 abup_get_partition_module(abup_uint32 base_addr);
#endif
