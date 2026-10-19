/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Drivers of pin control.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15895c
#define LOG_MODULE    AMOD_DRV

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_io.h>
#include <aic_cmdline.h>
#include <alog_api.h>
#ifdef CONFIG_PMIC
#include <pmic_reg_base.h>
#include <pmic_io.h>
#include <pd_pad_ctrl.h>
#endif
#include <pinlist.h>

#ifdef CONFIG_AIC_CHIP_GENESIS
/**
 * pin_gpio32_f3tof6() - Change pin SDMMC_RST/gpio32 function from 3 to 6
 *
 * For gx316, the default function of pin SDMMC_RST/gpio32 is 6(EIU_INT0).
 * It should bechanged to 6 (EIU_INT0).
 */
void pin_eiu_int0_f3tof6(void) // TODO: Modify this API name
{
    //AIC_WRITE32(REG_IOMUX_SDMMC_RST, BIT_FUNC_SEL(6));
    AIC_WRITE32(REG_IOMUX_SDMMC_D4, BIT_FUNC_SEL(4));
}

/**
 * pin_gpio32_f3tof6() - Change pin SDMMC_RST/gpio32 function from 6 to 3
 *
 * For gx316, the default function of pin SDMMC_RST/gpio32 is 6(EIU_INT0).
 * It should bechanged to 3 (gpio).
 */
void pin_eiu_int0_f6tof3(void) // TODO: Modify this API name
{
    //AIC_WRITE32(REG_IOMUX_SDMMC_RST, BIT_FUNC_SEL(3));
    AIC_WRITE32(REG_IOMUX_SDMMC_D4, BIT_FUNC_SEL(3));
}

/**
 * pmic_gpio9_f0tof2() - Change pmic gpio9 function from 0 to 2
 *
 * For gx316, the default function of pmic gpio9 is 0(gpio). It should be
 * changed to 2 (adc) when touch screen controller enabled.
 *
 * Return: AIC_SUCCESS or errno.
 */
s32 pmic_gpio9_f0tof2(void)
{
    u32 ret_val = AIC_SUCCESS;
#ifdef CONFIG_PMIC_I2C
    u32 rb_val;

    pmic_writel(PMIC_PG0_PAD_CTRL_IO_MUX_GPIO9, 0x2);
    rb_val = pmic_readl(PMIC_PG0_PAD_CTRL_IO_MUX_GPIO9);
    if (rb_val != 2) {
        aicx_error(0x056cae, "", "Switch pmic gpio0 from func0 to func2 failed\n");
        ret_val = AIC_EIO;
    }
#endif
    return ret_val;
}

/**
 * pincfg_emmc() - Additional pin setting for emmc support
 *
 * The function just do additional pin setting to support emmc,
 * Call this function after pin init.
 *
 * Return: AIC_SUCCESS or errno.
 */
s32 pincfg_emmc(void)
{
    u32 ret_val = AIC_SUCCESS;

    u32 pin_cfg[][2] = {
        {REG_PG7_PAD_CTRL_SDMMC_RST,
         BIT_PAD_DRV(1)|BIT_PAD_PX|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z},
        {REG_IOMUX_SDMMC_D4,
         BIT_FUNC_SEL(0)},
        {REG_PG7_PAD_CTRL_SDMMC_D4,
         BIT_PAD_DRV(1)|BIT_PAD_WPU|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z},
        {REG_IOMUX_SDMMC_D5,
         BIT_FUNC_SEL(0)},
        {REG_IOMUX_SDMMC_D6,
         BIT_FUNC_SEL(0)},
        {REG_IOMUX_SDMMC_D7,
         BIT_FUNC_SEL(0)},
        {REG_PG7_PAD_CTRL_SDMMC_D7,
         BIT_PAD_DRV(1)|BIT_PAD_WPU|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z},
    };

    for (int i = 0; i < sizeof(pin_cfg) >> 3; i++)
        AIC_WRITE32(pin_cfg[i][0], pin_cfg[i][1]);


    return ret_val;
}
#endif
/**
 * pin_func_config() - Change pin function
 *
 * @pin_reg: pin register address
 * @func: pin function, only support 0 ~ 6
 */
void pin_func_config(u32 pin_reg, u32 func)
{
    AIC_WRITE32(pin_reg, BIT_FUNC_SEL(func));
}

/**
 * pincfg_jtag() - Config jtag pin
 *
 * The function switch jtag pin to other functoin in user mode.
 *
 */
void pincfg_jtag(void)
{
    if (aic_is_user_mode()) {
#ifdef CONFIG_AIC_CHIP_GENESIS
        AIC_WRITE32(REG_IOMUX_DAP_TCK, BIT_FUNC_SEL(6));
        AIC_WRITE32(REG_IOMUX_DAP_TMS, BIT_FUNC_SEL(6));
        AIC_WRITE32(REG_IOMUX_DAP_TDO, BIT_FUNC_SEL(6));
        AIC_WRITE32(REG_IOMUX_DAP_TDI, BIT_FUNC_SEL(6));
        AIC_WRITE32(REG_IOMUX_DAP_TRST, BIT_FUNC_SEL(6));
#else
        //jtag pin used for GPIO
        //AIC_WRITE32(REG_PG5_PAD_CTRL_SWD_CLK, BIT_FUNC_SEL(2));
        //AIC_WRITE32(REG_PG5_PAD_CTRL_SWD_IO, BIT_FUNC_SEL(2));
#endif
    }
}

