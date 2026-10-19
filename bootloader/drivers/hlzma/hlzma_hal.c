/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Hardware abstraction layer(HAL) of hardware lzma driver.
 */

#include <aic_type.h>
#include <aic_errno.h>
#include "hlzma_phy.h"

#define xHLZMA_DEBUG

#ifdef HLZMA_DEBUG
#define HLZMA_LOG_NUM  10
u32 g_dbghlzma[HLZMA_LOG_NUM] = {0};
u32 g_dbghlzma_cnt = 0;
#endif

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
    s32 ret_val;
    hlzma_setting_t setting;
#ifdef HLZMA_DEBUG
    u32 ticks;
#endif
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

