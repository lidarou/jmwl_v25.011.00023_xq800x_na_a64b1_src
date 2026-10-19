
/*****************************************************************************/
/*                                                                           */
/*         Copyright 2022 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-08-29 15:15:58
 */

#ifndef __WDT_R2P0_H__
#define __WDT_R2P0_H__

#define WDT_REG_WDT_IP_INFO                                                     (PMIC_BASE_WDT + 0x0)
#define WDT_REG_WDT_GLB_CTRL                                                    (PMIC_BASE_WDT + 0x4)
#define WDT_REG_WDT_UPDATE                                                      (PMIC_BASE_WDT + 0x8)
#define WDT_REG_WDT_INT_EN                                                      (PMIC_BASE_WDT + 0xc)
#define WDT_REG_WDT_INT_STATUS_RAW                                              (PMIC_BASE_WDT + 0x10)
#define WDT_REG_WDT_INT_STATUS                                                  (PMIC_BASE_WDT + 0x14)
#define WDT_REG_WDT_INT_CLR                                                     (PMIC_BASE_WDT + 0x18)
#define WDT_REG_WDT_IRQ_CNT                                                     (PMIC_BASE_WDT + 0x1c)
#define WDT_REG_WDT_RST_CNT                                                     (PMIC_BASE_WDT + 0x20)
#define WDT_REG_WDT_LOCK                                                        (PMIC_BASE_WDT + 0x24)
#define WDT_REG_WDT_DEBUG_STATUS                                                (PMIC_BASE_WDT + 0x28)
#define WDT_REG_WDT_RST_INT_STATIC_DATA                                         (PMIC_BASE_WDT + 0x2c)
#define WDT_REG_WDT_RST_INT_STATIC_DATA_CLR                                     (PMIC_BASE_WDT + 0x30)
#define WDT_REG_WDT_GLB_CTRL_SET                                                (PMIC_BASE_WDT + 0x1004)
#define WDT_REG_WDT_UPDATE_SET                                                  (PMIC_BASE_WDT + 0x1008)
#define WDT_REG_WDT_INT_EN_SET                                                  (PMIC_BASE_WDT + 0x100c)
#define WDT_REG_WDT_INT_CLR_SET                                                 (PMIC_BASE_WDT + 0x1018)
#define WDT_REG_WDT_IRQ_CNT_SET                                                 (PMIC_BASE_WDT + 0x101c)
#define WDT_REG_WDT_RST_CNT_SET                                                 (PMIC_BASE_WDT + 0x1020)
#define WDT_REG_WDT_LOCK_SET                                                    (PMIC_BASE_WDT + 0x1024)
#define WDT_REG_WDT_GLB_CTRL_CLR                                                (PMIC_BASE_WDT + 0x2004)
#define WDT_REG_WDT_UPDATE_CLR                                                  (PMIC_BASE_WDT + 0x2008)
#define WDT_REG_WDT_INT_EN_CLR                                                  (PMIC_BASE_WDT + 0x200c)
#define WDT_REG_WDT_INT_CLR_CLR                                                 (PMIC_BASE_WDT + 0x2018)
#define WDT_REG_WDT_IRQ_CNT_CLR                                                 (PMIC_BASE_WDT + 0x201c)
#define WDT_REG_WDT_RST_CNT_CLR                                                 (PMIC_BASE_WDT + 0x2020)
#define WDT_REG_WDT_LOCK_CLR                                                    (PMIC_BASE_WDT + 0x2024)

/*WDT_REG_WDT_IP_INFO*/
#define BIT_WDT_IP_MAIN_VERSION(x)                                              (((x) & 0xff) << 8)
#define BIT_WDT_IP_SUB_VERSION(x)                                               (((x) & 0xff))

/*WDT_REG_WDT_GLB_CTRL*/
#define BIT_WDT_WDT_EN                                                          BIT(0)

/*WDT_REG_WDT_UPDATE*/
#define BIT_WDT_WDT_UPDATE                                                      BIT(0)

/*WDT_REG_WDT_INT_EN*/
#define BIT_WDT_WDT_RST_INT_EN                                                  BIT(1)
#define BIT_WDT_WDT_IRQ_INT_EN                                                  BIT(0)

/*WDT_REG_WDT_INT_STATUS_RAW*/
#define BIT_WDT_WDT_RST_INT_STATUS_RAW                                          BIT(1)
#define BIT_WDT_WDT_IRQ_INT_STATUS_RAW                                          BIT(0)

/*WDT_REG_WDT_INT_STATUS*/
#define BIT_WDT_WDT_RST_INT_STATUS                                              BIT(1)
#define BIT_WDT_WDT_IRQ_INT_STATUS                                              BIT(0)

/*WDT_REG_WDT_INT_CLR*/
#define BIT_WDT_WDT_RST_INT_CLR                                                 BIT(1)
#define BIT_WDT_WDT_IRQ_INT_CLR                                                 BIT(0)

/*WDT_REG_WDT_IRQ_CNT*/
#define BIT_WDT_WDT_IRQ_CNT(x)                                                  (((x) & 0xffffffff))

/*WDT_REG_WDT_RST_CNT*/
#define BIT_WDT_WDT_RST_CNT(x)                                                  (((x) & 0xffffffff))

/*WDT_REG_WDT_LOCK*/
#define BIT_WDT_WDT_LOCK(x)                                                     (((x) & 0xffffffff))

/*WDT_REG_WDT_DEBUG_STATUS*/
#define BIT_WDT_WDT_ACTUAL_CNT(x)                                               (((x) & 0xffffffff))

/*WDT_REG_WDT_RST_INT_STATIC_DATA*/
#define BIT_WDT_WDT_RST_INT_EN_STATIC_DATA(x)                                   (((x) & 0xffff) << 16)
#define BIT_WDT_WDT_RST_INT_STATUS_RAW_STATIC_DATA(x)                           (((x) & 0xffff))

/*WDT_REG_WDT_RST_INT_STATIC_DATA_CLR*/
#define BIT_WDT_WDT_RST_INT_STATIC_DATA_CLR                                     BIT(0)

/*WDT_REG_WDT_GLB_CTRL_SET*/
#define BIT_WDT_WDT_EN                                                          BIT(0)

/*WDT_REG_WDT_UPDATE_SET*/
#define BIT_WDT_WDT_UPDATE                                                      BIT(0)

/*WDT_REG_WDT_INT_EN_SET*/
#define BIT_WDT_WDT_RST_INT_EN                                                  BIT(1)
#define BIT_WDT_WDT_IRQ_INT_EN                                                  BIT(0)

/*WDT_REG_WDT_INT_CLR_SET*/
#define BIT_WDT_WDT_RST_INT_CLR                                                 BIT(1)
#define BIT_WDT_WDT_IRQ_INT_CLR                                                 BIT(0)

/*WDT_REG_WDT_IRQ_CNT_SET*/
#define BIT_WDT_WDT_IRQ_CNT(x)                                                  (((x) & 0xffffffff))

/*WDT_REG_WDT_RST_CNT_SET*/
#define BIT_WDT_WDT_RST_CNT(x)                                                  (((x) & 0xffffffff))

/*WDT_REG_WDT_LOCK_SET*/
#define BIT_WDT_WDT_LOCK(x)                                                     (((x) & 0xffffffff))

/*WDT_REG_WDT_GLB_CTRL_CLR*/
#define BIT_WDT_WDT_EN                                                          BIT(0)

/*WDT_REG_WDT_UPDATE_CLR*/
#define BIT_WDT_WDT_UPDATE                                                      BIT(0)

/*WDT_REG_WDT_INT_EN_CLR*/
#define BIT_WDT_WDT_RST_INT_EN                                                  BIT(1)
#define BIT_WDT_WDT_IRQ_INT_EN                                                  BIT(0)

/*WDT_REG_WDT_INT_CLR_CLR*/
#define BIT_WDT_WDT_RST_INT_CLR                                                 BIT(1)
#define BIT_WDT_WDT_IRQ_INT_CLR                                                 BIT(0)

/*WDT_REG_WDT_IRQ_CNT_CLR*/
#define BIT_WDT_WDT_IRQ_CNT(x)                                                  (((x) & 0xffffffff))

/*WDT_REG_WDT_RST_CNT_CLR*/
#define BIT_WDT_WDT_RST_CNT(x)                                                  (((x) & 0xffffffff))

/*WDT_REG_WDT_LOCK_CLR*/
#define BIT_WDT_WDT_LOCK(x)                                                     (((x) & 0xffffffff))

#endif /*__WDT_R2P0_H__*/
