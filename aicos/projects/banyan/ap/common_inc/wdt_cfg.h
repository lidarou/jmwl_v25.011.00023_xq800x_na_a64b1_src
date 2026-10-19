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
#define AON_WDT_EB                      REG_CKM_EN_GLB_TOP_TOP_WDT_MCU_EB
#define WDT_REB_BASE                    REG_BASE_TOP_WDT_MCU
#define REG_GLB_SW_RST_WDT              REG_CKM_EN_GLB_TOP_TOP_WDT_MCU_SW_RST_GLB_CFG
#define BIT_GLB_SW_RST_GLB_CLK_AON_WDT  BIT_CKM_EN_GLB_TOP_SW_RST_GLB_TOP_WDT_MCU
#define REG_AON_GLB_WDT_INT_REQ_ENABLE  REG_TOP_GLB_WDT_INT_REQ_ENABLE
#define IRQ_WDG                         IRQ_TOP_WDT_MCU
#define BIT_AON_GLB_WDT0_EB             BIT_CKM_EN_GLB_TOP_TOP_WDT_MCU_EB

#define WDT_PING_PERIOD_TIME    3000
#define WDT_IRQ_TIMEOUT         9000
#define WDT_RST_TIMEOUT         15000

#include <sys_nd_glb.h>
#define WDT_RETENTION_REG               REG_SYS_ND_GLB_WDT_CFG_REG0
#define WDT_RETENTION_IRQ_BIT           BIT(2)
#define WDT_RETENTION_RST_BIT           BIT(1)
#define WDT_RETENTION_EN_BIT            BIT(0)
#define WDT_RETENTION_IRQ_CNT_2S(x)     (((x) & 0xff) << 3)
#define WDT_RETENTION_RST_CNT_2S(x)     (((x) & 0xff) << 11)
#endif /* __WDT_CFG_H__ */
