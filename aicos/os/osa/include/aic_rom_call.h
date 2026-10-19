/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#ifndef __AIC_ROM_CALL_H__
#define __AIC_ROM_CALL_H__
#include "aos_utl.h"
#include "alog_idx_api.h"
#include "aic_call.h"

#ifndef CONFIG_ROM_BIN_BUILD
#include "aic_assert.h"
#include "aipc_api.h"
#include "aic_osa.h"
#endif

/* Ensure that standard C is used to process the API information.  */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
extern __attribute__((section(".rom_call_aos_memcpy"))) void (*s_aos_memcpy)(void *dest_ptr, const void *src_ptr, u32 copy_count);

static inline void ROM_CALL_DEF(aos_memcpy)(void *dest_ptr, const void *src_ptr, u32 copy_count)
{
        s_aos_memcpy(dest_ptr, src_ptr, copy_count);
}

extern __attribute__((section(".rom_call_aos_memset"))) void (*s_aos_memset)(void *dest_ptr, u8 value, u32 count);

static inline void ROM_CALL_DEF(aos_memset)(void *dest_ptr, u8 value, u32 count)
{
        s_aos_memset(dest_ptr, value, count);
}

extern __attribute__((section(".rom_call_ablk_get_block"))) int (*s_ablk_get_block)(ABRR_HANDLE handle, size_t size, ABLK_T *blk, u32 timeout);

static inline int ROM_CALL_DEF(ablk_get_block)(ABRR_HANDLE handle, size_t size, ABLK_T *blk, u32 timeout)
{
        return s_ablk_get_block(handle, size, blk, timeout);
}

extern __attribute__((section(".rom_call_ablk_recycle_block_ex"))) void (*s_ablk_recycle_block_ex)(ABRR_HANDLE handle, ABLK_T *blk, u16 line);

static inline void ROM_CALL_DEF(ablk_recycle_block_ex)(ABRR_HANDLE handle, ABLK_T *blk, u16 line)
{
        s_ablk_recycle_block_ex(handle, blk, line);
}

extern __attribute__((section(".rom_call_ablk_release_block_ex"))) void (*s_ablk_release_block_ex)(ABRR_HANDLE handle, ABLK_T *blk, u16 line);

static inline void ROM_CALL_DEF(ablk_release_block_ex)(ABRR_HANDLE handle, ABLK_T *blk, u16 line)
{
        s_ablk_release_block_ex(handle, blk, line);
}

extern __attribute__((section(".rom_call_aic_sys_assert"))) void (*s_aic_sys_assert)(char *fmt, ...);

#define rc_aic_sys_assert(fmt, ...) \
        do {                         \
                s_aic_sys_assert(fmt, ##__VA_ARGS__); \
        } while(0)

extern __attribute__((section(".rom_call_aos_disable_irq"))) u32 (*s_aos_disable_irq)(void);

static inline u32 ROM_CALL_DEF(aos_disable_irq)(void)
{
        return s_aos_disable_irq();
}

extern __attribute__((section(".rom_call_aos_restore_irq"))) void (*s_aos_restore_irq)(u32 save);

static inline void ROM_CALL_DEF(aos_restore_irq)(u32 save)
{
        s_aos_restore_irq(save);
}

extern __attribute__((section(".rom_call_aos_dma_malloc_debug"))) void *(*s_aos_dma_malloc_debug)(u32 size, unsigned int file_name, u32 line);

static inline void *ROM_CALL_DEF(aos_dma_malloc_debug)(u32 size, unsigned int file_name, u32 line)
{
        return s_aos_dma_malloc_debug(size, file_name, line);
}

extern __attribute__((section(".rom_call_aos_free_debug"))) u32 (*s_aos_free_debug)(u8 *mem_ptr, unsigned int file_name, u32 line);

static inline u32 ROM_CALL_DEF(aos_free_debug)(u8 *mem_ptr, unsigned int file_name, u32 line)
{
        return s_aos_free_debug(mem_ptr, file_name, line);
}

extern __attribute__((section(".rom_call_aos_malloc_prefer_bytepool"))) void *(*s_aos_malloc_prefer_bytepool)(u32 size, bool zero, unsigned int file_name, u32 line, boolean b_strict);
static inline void *ROM_CALL_DEF(aos_malloc_prefer_bytepool)(u32 size, bool zero, unsigned int file_name, u32 line, boolean b_strict)
{
        return s_aos_malloc_prefer_bytepool(size, zero, file_name, line, b_strict);
}

extern __attribute__((section(".rom_call_aos_link_mem_debug"))) u32 (*s_aos_link_mem_debug)(u8 *mem_ptr, unsigned int file_name, u32 line);
static inline u32 ROM_CALL_DEF(aos_link_mem_debug)(u8 *mem_ptr, unsigned int file_name, u32 line)
{
        return s_aos_link_mem_debug(mem_ptr, file_name, line);
}

extern __attribute__((section(".rom_call_dcache_inv_by_range"))) void (*s_dcache_inv_by_range)(u32 addr, size_t length);

static inline void ROM_CALL_DEF(dcache_inv_by_range)(u32 addr, size_t length)
{
        s_dcache_inv_by_range(addr, length);
}

extern __attribute__((section(".rom_call_dcache_clean_by_range"))) void (*s_dcache_clean_by_range)(u32 addr, size_t length);

static inline void ROM_CALL_DEF(dcache_clean_by_range)(u32 addr, size_t length)
{
        s_dcache_clean_by_range(addr, length);
}

extern __attribute__((section(".rom_call_aic_sys_is_assert"))) boolean (*s_aic_sys_is_assert)(void);

static inline boolean ROM_CALL_DEF(aic_sys_is_assert)(void)
{
        return s_aic_sys_is_assert();
}

#endif

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_ROM_CALL_H__ */

