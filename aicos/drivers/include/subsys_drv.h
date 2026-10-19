
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the header file of the subsys driver module.
 */
#ifndef __SUBSYS_DRV_H__
#define __SUBSYS_DRV_H__

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_osa.h>
#include <subsys_header.h>

typedef struct region_arear {
    u8 *buffer;
    size_t size;
} region_arear_t;

typedef struct subsys_dump {
    char    *region_name;
    base_t  region_base;
    size_t  region_size;
    u32     region_attr;
} subsys_dump_t;

/* api define */
void *subsys_create(u8 sys, u32 region_num, subsys_region_t *region_tab);
void subsys_destroy(void *sys_handle);

int subsys_read_lock(void *sys_handle);
void subsys_read_unlock(void *sys_handle);

int subsys_write_lock(void *sys_handle);
void subsys_write_unlock(void *sys_handle);

int subsys_read_region(void *sys_handle, u8 *buffer, size_t size);
int subsys_write_region(void *sys_handle, u8 *buffer, size_t size);
int subsys_set_write_region(void *sys_handle, char *name);
void subsys_get_write_region(void *sys_handle, region_arear_t *arear);

int subsys_drv_stop(void *sys_handle);
int subsys_drv_start(void *sys_handle);
int subsys_drv_poweron(void *sys_handle);
int subsys_drv_poweroff(void *sys_handle);
bool subsys_drv_is_poweroff(void *sys_handle);

#endif /* __SUBSYS_DRV_H__ */

