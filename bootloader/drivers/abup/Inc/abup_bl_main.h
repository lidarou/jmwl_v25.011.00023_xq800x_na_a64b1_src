/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_bl_main.h
* Description: abup_bl_main.c头文件
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef _ABUP_MAIN_H
#define _ABUP_MAIN_H

#define __MULTI_PACKAGE__
#include "abup_typedef.h"
#include "abup_hal_uart.h"

typedef struct multi_head_info{
	abup_uint32 multi_bin_offset;
	abup_uint32 multi_bin_address;
	abup_uint8 multi_bin_method;
	abup_uint8 multi_bin_resever[7];
}multi_head_info_patch;



//Error code

#define ABUP_FUDIFFNET_ERROR_NONE                                               (0)
#define ABUP_FUDIFFNET_ERROR_INSUFFICIENT_WORKING_BUF                           (-100)

//Updating related
#define ABUP_FUDIFFNET_ERROR_UPDATE_ERROR_START                                 (-600)
#define ABUP_FUDIFFNET_ERROR_UPDATE_ERROR_END                                   (-699)


abup_extern int AbupProcedure(void);
abup_extern void abup_patch_progress(abup_uint16 total,abup_uint16 current);
abup_extern abup_int32 abup_bl_write_block(abup_uint8 type,abup_uint8* src, abup_uint32 start, abup_uint32 datasize);
abup_extern abup_int32 abup_bl_read_block(abup_uint8 type,abup_uint8* dest, abup_uint32 start, abup_uint32 datasize);
abup_extern void abup_bl_progress(abup_int32 percent);
abup_extern abup_uint8* abup_bl_get_working_buffer(void);
abup_extern abup_uint abup_bl_get_working_buffer_len(void);
abup_extern abup_uint32 abup_bl_get_app_base(void);
abup_extern abup_uint32 abup_bl_get_diff_param_size(void);
abup_extern abup_uint32 abup_bl_get_delta_base(void);
abup_extern abup_uint32 abup_hal_get_app_addr(void);

#if defined(ABUP_CHECK_PATCH_FILE)&&(ABUP_CHECK_PATCH_FILE == 1)
#if defined(ABUP_FOTA_ALGORITHM)&&(ABUP_FOTA_ALGORITHM == 0)||defined(WIN32) || defined(LINUX)
abup_extern abup_bool AbupLusunCheckPatchFile(void);
#endif
#if defined(ABUP_FOTA_ALGORITHM)&&(ABUP_FOTA_ALGORITHM != 0)||defined(WIN32) || defined(LINUX)
abup_extern abup_bool AbupWosunCheckPatchFile(void);
#endif
abup_extern abup_bool AbupCheckPatchFile(abup_uint32 patchsize);
#endif

#endif
