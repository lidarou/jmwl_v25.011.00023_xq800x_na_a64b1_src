/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __WDT_CFG_H__
#define __WDT_CFG_H__

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_reg_base.h>
#include <irq_chip.h>

/*sub sys wdt base redefine*/
#define AON_WDT_EB                      REG_AON_GLB_WDT0_EB
#define WDT_REB_BASE                    REG_BASE_AON_WDT0
#define REG_GLB_SW_RST_WDT              REG_TOP_GLB_SW_RST_WDT0
#define BIT_GLB_SW_RST_GLB_CLK_AON_WDT  BIT_TOP_GLB_SW_RST_GLB_CLK_AON_WDT0


#define WDT_PING_PERIOD_TIME    3000
#define WDT_IRQ_TIMEOUT         9000
#define WDT_RST_TIMEOUT         15000

#endif /* __WDT_CFG_H__ */
