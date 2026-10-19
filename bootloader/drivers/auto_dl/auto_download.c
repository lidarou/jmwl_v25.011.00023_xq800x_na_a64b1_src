/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#include <auto_dl/auto_dl_cfg.h>
#include <boot_mode.h>
#include <cache.h>
#include "console/console.h"
#include "initcall.h"
#include <mpu.h>
#include "platform/platform.h"
#include "platform/boot.h"
#include <aic_type.h>
#include "gpio_inc.h"

static int auto_download_init(void)
{
    log_print("auto_download_init, boot_mode = %d\n", get_boot_mode());
    if (get_boot_mode() != AUTODL_BOOT)
        return 0;

    AIC_WRITE32(auto_dl.gpio_pin_base, auto_dl.gpio_func_sel);
    aic_gpio_request(auto_dl.gpio_num);
    aic_gpio_output(auto_dl.gpio_num,GPIO_OUTPUT_LVL_LOW);

    mpu_disable();
    dcache_invcc_all();
    icache_inv_all();
    cache_disable();
    boot_platform_quit((struct boot_arm_vector_table *)0x0);
    return 0;
}

core_init(auto_download_init);
