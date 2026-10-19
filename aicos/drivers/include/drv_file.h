/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    temp use drv_file.h befor file driver ready.
 */
#ifndef __DRV_FILE_H__
#define __DRV_FILE_H__

#include <aic_type.h>

#ifdef WIN32
#define CONFIG_FILE_HAL_DRIVE

static __inline int fmd_close(u32 handle) {return 0;}
static __inline int fmd_open(char *name, u32 *handle) {return 0;}
static __inline int fmd_read(u32 handle, u32 offset, u32 size, u8 *buf)  {return size;}
static __inline int fmd_write(u32 handle, u32 offset, u32 size, u8 *buf) {return size;}
static __inline int fmd_get_physical_address(u32 handle, u32 offset, u32 *retaddr) {return 0;}
#else
#include <fmd.h>
#endif

#ifdef CONFIG_FILE_HAL_DRIVE
#define MAX_FIE_PATH_LEN    128
#define CUR_DRV             0xFF

/* Normal file - No read/write restrictions. */
#define NORMAL_MODE         0x00

/* temp use drv_usart.h befor uart driver ready. */
void *file_open(char *filename, char *mode);
void file_close(void *handle);
size_t file_read(void *handle, void *buffer, size_t size);
size_t file_write(void *handle, const void *buffer, size_t size);
int file_access(const char *filename, int mode);
int file_mkdir(const char *path);
int file_seek(void *handle, u32 offset);
void *file_get_mem_addr(void *handle, u32 offset);
int file_rm_all_dir(const char *path);
int file_get_free(int drv, u64 *free_bytes, u64 *total_bytes);
#endif

#endif /* __DRV_FILE_H__ */
