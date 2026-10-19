
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the loader header file of the subsys.
 */
#ifndef __SUBSYS_LOADER_H__
#define __SUBSYS_LOADER_H__

#include <aic_type.h>
#include <subsys_drv.h>

typedef struct subsys_loader {
    u8                  sys;
    u8                  region_num;
    u16                 bootcode_size;
    subsys_region_t     *region_tbl;
    u32                 *bootcode;
    void                *subsys;
    char                *name;
} subsys_loader_t;

void *subsys_loader_create(u8 sys, char *name);
void subsys_loader_release(subsys_loader_t *loader);
void subsys_loader_load(subsys_loader_t *loader);
void subsys_loader_start(subsys_loader_t *loader);
void subsys_loader_power_on(subsys_loader_t *loader);
void subsys_loader_power_off(subsys_loader_t *loader);
void subsys_loader_dump_sys(u8 sys);

#define subsys_loader_stop(ld)          subsys_drv_stop((ld)->subsys)
#define subsys_loader_is_poweroff(ld)   subsys_drv_is_poweroff((ld)->subsys)

#endif /* __SUBSYS_LOADER_H__ */

