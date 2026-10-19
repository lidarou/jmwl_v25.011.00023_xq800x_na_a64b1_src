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
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_io.h>
#include <flash_config.h>
#include <flash_pin_group.h>
#include <initcall.h>
#include <pinlist.h>
#include "console/console.h"
#include <pmic_io.h>
#include <timer.h>
#include <pin_cfg.h>

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

int pin_bl_init(void)
{
    const u32 (*pin_cfg)[2] = AIC_NULL;
    u32 i, size;

    pin_cfg = pin_bl_cfg;
    size = sizeof(pin_bl_cfg)/8;

    for (i = 0; i < size; i++) {
        AIC_WRITE32(pin_cfg[i][0], pin_cfg[i][1]);
    }

    return AIC_SUCCESS;
}
core_init(pin_bl_init);

#if (QSPI_NOR_DEVICE_NUM == 2)
#if defined(CONFIG_MBL) || defined(CONFIG_MDL)
int extflash_pin_init(void)
{
    u32 i;
    struct flash_pin *pin_cfg = extflash_pin_group[FLASH_PIN_GROUP_INDEX];

#ifdef AUX_CLK0_RESERVED_REG1
    /* clear 0x410803e4 bit0 to 0 */
    AIC_WRITE32(AUX_CLK0_RESERVED_REG1 + 0x2000, BIT(0));
#endif

    for (i = 0; i < FLASH_PIN_NUM; i++)
        AIC_WRITE32(pin_cfg[i].reg_adr, pin_cfg[i].reg_val);

    return 0;
}
service_init(extflash_pin_init);

#ifdef FLASH_LDO3V3_CFG_REG
int extflash_ldo_init(void)
{
    u32 val;

    pmic_writel(FLASH_LDO3V3_CFG_REG, FLASH_LDO3V3_CFG_VAL);
    val = pmic_readl(FLASH_LDO3V3_BOUND_REG);
    pmic_writel(FLASH_LDO3V3_BOUND_REG, (val & ~FLASH_LDO3V3_POWER_DOWN));
    pmic_writel(FLASH_LDO_CFG_UPDATE_REG, FLASH_LDO_UPDATE);

    return 0;
}
service_init(extflash_ldo_init);
#endif
#endif

#ifdef CONFIG_BL
int extflash_ldo_sleep_init(void)
{
#ifdef FLASH_BUK0_BOUND_REG
    /* vddcore always on */
    pmic_writel(FLASH_BUK0_BOUND_REG,
                (pmic_readl(FLASH_BUK0_BOUND_REG) & ~FLASH_BUK0_SLP_BOUND));
    pmic_writel(FLASH_LDO_CFG_UPDATE_REG, FLASH_LDO_UPDATE);
#endif

#ifdef FLASH_LDO3V3_BOUND_REG
    /* flash ldo always on */
    pmic_writel(FLASH_LDO3V3_BOUND_REG,
                (pmic_readl(FLASH_LDO3V3_BOUND_REG) & ~FLASH_LDO3V3_SLP_BOUND));
    pmic_writel(FLASH_LDO_CFG_UPDATE_REG, FLASH_LDO_UPDATE);
#endif

    return 0;
}
service_init(extflash_ldo_sleep_init);
#endif
#endif
