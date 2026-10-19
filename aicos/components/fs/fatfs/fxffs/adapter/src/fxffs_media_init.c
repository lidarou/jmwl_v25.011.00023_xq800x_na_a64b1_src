/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system init Interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09f25c
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <fx_api.h>
#include "fxffs.h"

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fxffs_media_system_init -
 *  This function initializes the various control data structures for
 *  the ffs component.
 *
 * Parameter:
 *
 * Return:
 *
 */
void fxffs_media_system_init(void);

/**
 * fxffs_time_set -
 *  This function set the file system date and time.
 *
 * Parameter:
 * @time: file system time set.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_time_set(void *time)
{
    aos_systime *systime = (aos_systime *)time;
    u32 ret;

    do {
        ret = fx_system_date_set(systime->year, systime->month, systime->day);
        if (ret != FX_SUCCESS) {
            break;
        }

        ret = fx_system_time_set(systime->hour, systime->minute, systime->second);
        if (ret != FX_SUCCESS) {
            break;
        }
    } while(0);

    return ret;
}

/**
 * fxffs_time_get -
 *  This function get the file system date and time.
 *
 * Parameter:
 * @time: file system time.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_time_get(void *time)
{
    aos_systime *systime = (aos_systime *)time;
    u32 ret;

    do {
        ret = fx_system_date_get(&systime->year, &systime->month, &systime->day);
        if (ret != FX_SUCCESS) {
            break;
        }

        ret = fx_system_time_get(&systime->hour, &systime->minute, &systime->second);
        if (ret != FX_SUCCESS) {
            break;
        }
    } while(0);

    return ret;
}

FXFFS_MEDIA_INIT_OPS s_fxffs_media_init_ops = {
    AOS_FALSE,
    fxffs_media_system_init,
    fxffs_time_set,
    fxffs_time_get,
};

/**
 * fxffs_media_system_init -
 *  This function initializes the various control data structures for
 *  the ffs component.
 *
 * Parameter:
 *
 * Return:
 *
 */
void fxffs_media_system_init(void)
{
    if (!s_fxffs_media_init_ops.initd) {
        fx_system_initialize();
        s_fxffs_media_init_ops.initd = AOS_TRUE;
    }
}

