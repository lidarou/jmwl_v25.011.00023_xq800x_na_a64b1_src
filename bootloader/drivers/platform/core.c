/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

#include <fota.h>
#include "console/console.h"
#include "platform/platform_cfg.h"
#include "aic_errno.h"
#include "efuse.h"
#include <assert.h>
#include <secure_efuse_cfg.h>

int platform_secboot_init(SECBOOT_AUTH auth)
{
    u32 blk_sec_data = 0;
    int ret = 0;

    platform.secure_boot.secboot_authorization = auth;

    ret = aic_efuse_read(SEC_BOOT_EN_BLK, &blk_sec_data);

    if (ret)
        goto out;

    if (blk_sec_data & BIT(SEC_BOOT_EN_BIT))
        platform.secure_boot.secure_boot_enable = true;
    else
        platform.secure_boot.secure_boot_enable = false;

out:
    return ret;
}

uint32_t platform_get_dl_type(void)
{
    return platform.bp->dl_type;
}

int platform_boot_mode_enterpoint_register(
                    BOOT_MOD_ENTERPOINT enter,
                    enum BOOT_MOD mod)
{
    platform.boot_mode_enter[mod] = enter;
    platform.boot_mode_cnt++;

    return 0;
}

int platform_tick_register(u32 (*tick)(void))
{
    platform.get_tick = tick;
    platform.timer_count = 0;

    return 0;
}

static int platform_normal_boot(void)
{
    int i;

    for (i = NORMAL_BOOT + 1; i < BOOT_MOD_MAX; i++){
        if (platform.boot_mode_enter[i])
            platform.boot_mode_enter[i]();
    }

    platform.boot_mode = NORMAL_BOOT;
    return platform.boot_mode_enter[NORMAL_BOOT]();
}

int platform_do_boot(void)
{
    log_print("boot mode[%d] start running..", platform.boot_mode);

    if (platform.bp->boot_mode)
        return platform_normal_boot();

     return platform.boot_mode_enter[DOWNLOAD_BOOT]();
}

void platform_init(void)
{
    platform.get_tick = systick_default_get;
    platform.timer_count = 0;
}

