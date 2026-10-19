/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of pmic wdt module.
 *   It includes all the pmic wdt phy function api.
*/
#ifndef __WDT_PHY_H__
#define __WDT_PHY_H__
#include <pmic_wdt.h>

struct pmic_wdt_ops {
    void (*init)(void);
    void (*kick)(void);
    void (*mode_config)(PMIC_WDT_MODE mode);
    void (*timeout_set)(u32 irq_timeout_ms, u32 rst_timeout_ms);
    void (*start)(void);
    void (*stop)(void);
};

void pmic_phy_ops_register(struct pmic_wdt_ops *pmic_wdt_phy_ops);

#endif /* __WDT_PHY_H__ */
