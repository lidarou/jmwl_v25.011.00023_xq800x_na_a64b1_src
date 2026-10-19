/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the flash management device drivers.
 */

#ifndef __FMD_H__
#define __FMD_H__

#include <aic_type.h>

#define FMD_QSPI_NORFLASH 1
#define FMD_SSP_NORFLASH  2

int fmd_open(char *name, u32 *handle);
int fmd_close(u32 handle);
int fmd_read(u32 handle, u32 offset, u32 size, u8 *buf);
int fmd_write(u32 handle, u32 offset, u32 size, u8 *buf);
int fmd_erase(u32 handle, u32 offset, u32 size);
int fmd_lock(u32 handle, u32 offset, u32 size);
int fmd_unlock(u32 handle, u32 offset, u32 size);
boolean fmd_is_locked(u32 handle, u32 offset, u32 size);
int fmd_get_physical_address(u32 handle, u32 offset, u32 *retaddr);
int fmd_get_partition_size(u32 handle, u32 *retsize);
int fmd_get_partition_ofs(u32 handle, u32 *retofs);
int fmd_get_erase_size(u32 handle, u32 *retsize);
int fmd_enter_read_mode(u32 handle);
int fmd_partition_table_update(void);
int fmd_partition_table_check(void);
int fmd_erase_total_chip(u8 devno);
void fmd_erase_all_chips(void);
int fmd_get_old_partition_info(char *name, u32 *paddr, u32 *psize);
int fmd_get_dev_num(void);
u32 fmd_get_total_size(u8 devno);
u32 fmd_get_dev_handle(u8 devno);

#endif /* __FMD_H__ */

