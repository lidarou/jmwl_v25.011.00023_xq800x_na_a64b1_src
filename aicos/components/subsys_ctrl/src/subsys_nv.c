
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the subsys nv source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0673eb
#define LOG_MODULE    AMOD_SUBSYS

#include <aic_assert.h>
#include <aic_osa.h>
#include <subsys_loader.h>
#include <subsys_drv.h>

#ifdef CONFIG_NVRAM_SUPPORT
#include <aic_nvram_ap.h>
#endif

void subsys_nv_process(subsys_region_t *region, char *nv_file)
{
#ifdef CONFIG_NVRAM_SUPPORT
    nvram_ap_load_data();
#endif
}

