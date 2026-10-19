/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Hardware abstraction layer(HAL) of hardware lzma driver.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04a62c
#define LOG_MODULE    AMOD_DRV

#include <aic_init.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <timer_inc.h>
#include <mem_addr_check.h>
#include <dvfs_inc.h>
#include <qspi_inc.h>
#include <slp_inc.h>
#include "hlzma_phy.h"

#define HLZMA_DEBUG

#ifdef HLZMA_DEBUG
#define HLZMA_LOG_NUM  10
u32 g_dbghlzma[HLZMA_LOG_NUM] = {0};
u32 g_dbghlzma_cnt = 0;
#endif

aos_mutex_ptr hlzma_mutex = AOS_NULL;

/**
 * hlzma_dec_polling() - Do decompress and poll to done
 *
 * @in_addr: Start address of compressed data
 * @out_addr: Output address of decompressed data
 *
 * Decompress some data and polling until finish.
 *
 * Return: AIC_SUCCESS if no error;errno if there is error.
 *
 */
s32 hlzma_dec_polling(u32 in_addr, u32 out_addr)
{
    bool in_flash = FALSE;
    s32 ret_val, ret = AIC_ENODEV;
    hlzma_setting_t setting;
#ifdef HLZMA_DEBUG
    u32 ticks;
#endif
    if (aos_is_user_thread_context()) {
        if (hlzma_mutex)
            ret = aos_get_mutex(hlzma_mutex, AOS_WAIT_FOREVER);

        in_flash = mem_is_access_in_flash_area(in_addr);
        if (in_flash)
            qspi_lock();
        /* Get wake lock and keep voltage high when lzma working */
        aic_get_wakelock();
        aic_dvfs_force(RISE | SYSCPU_LZMA);
    }

    setting.in_addr = in_addr;
    setting.out_addr = out_addr;
#ifdef HLZMA_DEBUG
    ticks = aic_get_tick();
#endif
    hlzma_phy_en(AIC_TRUE);
    hlzma_phy_reset();
    hlzma_phy_int_disable();
    hlzma_phy_cfg(&setting);
    hlzma_phy_start();

    ret_val = hlzma_phy_polling_done();
#ifndef CONFIG_BYPASS_HLZMA_EB
    hlzma_phy_en(AIC_FALSE);
#endif
#ifdef HLZMA_DEBUG
    if (g_dbghlzma_cnt >= HLZMA_LOG_NUM)
        g_dbghlzma_cnt = 0;
    g_dbghlzma[g_dbghlzma_cnt++] = aic_get_tick() - ticks;
#endif

    if (aos_is_user_thread_context()) {
        if (in_flash)
            qspi_unlock();
        if (ret == AIC_SUCCESS)
            aos_put_mutex(hlzma_mutex);
        aic_dvfs_force(FALL | SYSCPU_LZMA);
        aic_put_wakelock();
    }

    return ret_val;
}

/**
 * hlzma_dec() - Do decompress and return after start
 *
 * @in_addr: Start address of compressed data
 * @out_addr: Output address of decompressed data
 *
 * Decompress some data and return immediately.
 *
 * Return: AIC_SUCCESS if no error;errno if there is error.
 *
 */
s32 hlzma_dec(u32 in_addr, u32 out_addr)
{
    hlzma_setting_t setting;

    setting.in_addr = in_addr;
    setting.out_addr = out_addr;

    hlzma_phy_en(AIC_TRUE);
    hlzma_phy_cfg(&setting);
    hlzma_phy_start();

    return AIC_SUCCESS;
}

s32 hlzma_init()
{
    hlzma_mutex = aos_create_mutex("hlzma-mutex");
    if (hlzma_mutex == AOS_NULL)
        aicx_error(0x009583, "", "Create hlzma mutex failed!\n");
    return AIC_SUCCESS;
}
AIC_DRV_P2_INIT_EXPORT(hlzma_init);
