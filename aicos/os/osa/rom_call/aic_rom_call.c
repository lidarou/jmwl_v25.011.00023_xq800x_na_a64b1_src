/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA event function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14d6f3
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include "aic_rom_call.h"
#include "cache.h"


void (*s_aos_memcpy)(void *dest_ptr, const void *src_ptr, u32 copy_count) = aos_memcpy;
void (*s_aos_memset)(void *dest_ptr, u8 value, u32 count) = aos_memset;
int (*s_ablk_get_block)(ABRR_HANDLE handle, size_t size, ABLK_T *blk, u32 timeout) = ablk_get_block;
void (*s_ablk_recycle_block_ex)(ABRR_HANDLE handle, ABLK_T *blk, u16 line) = ablk_recycle_block_ex;
void (*s_ablk_release_block_ex)(ABRR_HANDLE handle, ABLK_T *blk, u16 line) = ablk_release_block_ex;
void (*s_aic_sys_assert)(char *fmt, ...) = aic_sys_assert;
u32 (*s_aos_disable_irq)(void) = aos_disable_irq;
void (*s_aos_restore_irq)(u32 save) = aos_restore_irq;
void *(*s_aos_dma_malloc_debug)(u32 size, unsigned int file_name, u32 line) = aos_dma_malloc_debug;
u32 (*s_aos_free_debug)(u8 *mem_ptr, unsigned int file_name, u32 line) = aos_free_debug;
void *(*s_aos_malloc_prefer_bytepool)(u32 size, bool zero, unsigned int file_name, u32 line, boolean b_strict) = aos_malloc_prefer_bytepool;
u32 (*s_aos_link_mem_debug)(u8 *mem_ptr, unsigned int file_name, u32 line) = aos_link_mem_debug;
void (*s_dcache_inv_by_range)(u32 addr, size_t length) = dcache_inv_by_range;
void (*s_dcache_clean_by_range)(u32 addr, size_t length) = dcache_clean_by_range;
boolean (*s_aic_sys_is_assert)(void) = aic_sys_is_assert;
