#ifndef _ABUP_BL_FLASH_H
#define _ABUP_BL_FLASH_H

#include "abup_hal.h"

//#define BLOCK_SIZE_32

#ifdef ABUP_DEFAULT_SECTOR_SIZE
#define ABUP_TRUE_BLOCK_SIZE abup_hal_get_blocksize()
#define ABUP_BLOCK_SIZE abup_hal_get_blocksize()
#define ABUP_DIFF_PARAM_SIZE    abup_hal_get_blocksize()
#define NOR_PAGE_SIZE    abup_hal_get_blocksize()
#elif defined BLOCK_SIZE_64
#define ABUP_TRUE_BLOCK_SIZE (4*1024)
#define ABUP_BLOCK_SIZE (64*1024)
#define ABUP_DIFF_PARAM_SIZE    (64*1024)//64*1024//modified by hzy for mt2523,20161229
#elif defined BLOCK_SIZE_32
#define ABUP_TRUE_BLOCK_SIZE (32*1024)
#define ABUP_BLOCK_SIZE (32*1024)
#define ABUP_DIFF_PARAM_SIZE    (32*1024)
#elif defined BLOCK_SIZE_8
#define ABUP_TRUE_BLOCK_SIZE (4*1024)
#define ABUP_BLOCK_SIZE (8*1024)
#define ABUP_DIFF_PARAM_SIZE    (8*1024)
#else
#define ABUP_BLOCK_SIZE (4*1024)
#define ABUP_DIFF_PARAM_SIZE    (4*1024)//64*1024//modified by hzy for mt2523,20161229
#define ABUP_TRUE_BLOCK_SIZE (4*1024)
#endif

#ifndef NOR_PAGE_SIZE
#ifdef __SERIAL_FLASH_EN__
#define NOR_PAGE_SIZE             (512)         //512 bytes
#else
#define NOR_PAGE_SIZE             (4*1024) // (2048)        //2048 bytes//modified by hzy for reduce rom region,20170105
#endif
#endif

abup_uint32 abup_bl_flash_block_size(void);
abup_uint32 abup_bl_flash_backup_size(void);
abup_uint32 abup_bl_fota_get_block_size_length(abup_uint32 addr);
abup_uint32 abup_bl_flash_backup_base(void);
abup_uint32 abup_bl_flash_delta_base(void);
abup_uint32 abup_hal_get_delta_size(void);
abup_uint32 AbupGetFlashDiskSize(void);
abup_uint32 abup_bl_get_rtos_len(void);
abup_uint32 abup_bl_get_rtos_base(void);
abup_uint32 abup_bl_get_rom_base(void);
abup_int32 abup_bl_read_block(abup_uint8 type,abup_uint8* dest, abup_uint32 start, abup_uint32 size);
abup_int32 abup_bl_write_block(abup_uint8 type,abup_uint8* src, abup_uint32 start, abup_uint32 size);
abup_uint32 abup_bl_read_flash(abup_uint8 type,abup_uint32 offset, abup_uint8* dest,abup_uint32 size);
abup_int32 abup_bl_erase_block(abup_uint8 type,abup_uint32 addr);
abup_uint32 abup_bl_erase_delata(void);
void abup_bl_erase_backup_region(void);
abup_uint32 abup_bl_init_flash(void);
abup_int32 abup_bl_write_backup_region(abup_uint8* data_ptr, abup_uint32 len);
abup_int32  abup_bl_read_backup_region(abup_uint8* data_ptr, abup_uint32 len);
#endif 
