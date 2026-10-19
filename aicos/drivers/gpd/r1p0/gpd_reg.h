
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-05-18 13:13:20
 */

#ifndef __GPD_RF_H__
#define __GPD_RF_H__

#include <aic_reg_base.h>

#define REG_GPD_IP_INFO                                                     (REG_BASE_AON_GPD + 0x0)
#define REG_GPD_GLB_CTRL                                                    (REG_BASE_AON_GPD + 0x4)
#define REG_GPD_UPDATE                                                      (REG_BASE_AON_GPD + 0x8)
#define REG_GPD_INT_EN                                                      (REG_BASE_AON_GPD + 0xc)
#define REG_GPD_INT_STATUS_RAW                                              (REG_BASE_AON_GPD + 0x10)
#define REG_GPD_INT_STATUS                                                  (REG_BASE_AON_GPD + 0x14)
#define REG_GPD_INT_STATUS_CLR                                              (REG_BASE_AON_GPD + 0x18)
#define REG_GPD_EDGE_TYPE                                                   (REG_BASE_AON_GPD + 0x20)
#define REG_GPD_CNT(n)                                                      (REG_BASE_AON_GPD + ((n) * 0xc) + 0x24)
#define REG_GPD_EDGE(n)                                                     (REG_BASE_AON_GPD + ((n) * 0xc) + 0x28)
#define REG_GPD_CYCLE_CNT(n)                                                (REG_BASE_AON_GPD + ((n) * 0xc) + 0x2c)
#define REG_GPD_INT_EN_SET                                                  (REG_BASE_AON_GPD + 0x100c)
#define REG_GPD_INT_STATUS_CLR_SET                                          (REG_BASE_AON_GPD + 0x1018)
#define REG_GPD_INT_EN_CLR                                                  (REG_BASE_AON_GPD + 0x200c)
#define REG_GPD_INT_STATUS_CLR_CLR                                          (REG_BASE_AON_GPD + 0x2018)

/*REG_GPD_IP_INFO*/
#define BIT_GPD_IP_MAIN_VERSION(x)                                          (((x) & 0xff) << 8)
#define BIT_GPD_IP_SUB_VERSION(x)                                           (((x) & 0xff))

/*REG_GPD_GLB_CTRL*/
#define BIT_GPD3_EN                                                         BIT(3)
#define BIT_GPD2_EN                                                         BIT(2)
#define BIT_GPD1_EN                                                         BIT(1)
#define BIT_GPD0_EN                                                         BIT(0)

/*REG_GPD_UPDATE*/
#define BIT_GPD3_EDGE_CYCLE_CLR                                             BIT(11)
#define BIT_GPD2_EDGE_CYCLE_CLR                                             BIT(10)
#define BIT_GPD1_EDGE_CYCLE_CLR                                             BIT(9)
#define BIT_GPD0_EDGE_CYCLE_CLR                                             BIT(8)
#define BIT_GPD3_EDGE_CNT_CLR                                               BIT(7)
#define BIT_GPD2_EDGE_CNT_CLR                                               BIT(6)
#define BIT_GPD1_EDGE_CNT_CLR                                               BIT(5)
#define BIT_GPD0_EDGE_CNT_CLR                                               BIT(4)
#define BIT_GPD3_UPDATE                                                     BIT(0)
#define BIT_GPD2_UPDATE                                                     BIT(0)
#define BIT_GPD1_UPDATE                                                     BIT(0)
#define BIT_GPD0_UPDATE                                                     BIT(0)

/*REG_GPD_INT_EN*/
#define BIT_GPD3_INT_EN                                                     BIT(3)
#define BIT_GPD2_INT_EN                                                     BIT(2)
#define BIT_GPD1_INT_EN                                                     BIT(1)
#define BIT_GPD0_INT_EN                                                     BIT(0)

/*REG_GPD_INT_STATUS_RAW*/
#define BIT_GPD3_INT_STATUS_RAW                                             BIT(3)
#define BIT_GPD2_INT_STATUS_RAW                                             BIT(2)
#define BIT_GPD1_INT_STATUS_RAW                                             BIT(1)
#define BIT_GPD0_INT_STATUS_RAW                                             BIT(0)

/*REG_GPD_INT_STATUS*/
#define BIT_GPD_INT_MASK                                                    (0xf)
#define BIT_GPD3_INT_STATUS                                                 BIT(3)
#define BIT_GPD2_INT_STATUS                                                 BIT(2)
#define BIT_GPD1_INT_STATUS                                                 BIT(1)
#define BIT_GPD0_INT_STATUS                                                 BIT(0)

/*REG_GPD_INT_STATUS_CLR*/
#define BIT_GPD3_INT_STATUS_CLR                                             BIT(3)
#define BIT_GPD2_INT_STATUS_CLR                                             BIT(2)
#define BIT_GPD1_INT_STATUS_CLR                                             BIT(1)
#define BIT_GPD0_INT_STATUS_CLR                                             BIT(0)

/*REG_GPD_EDGE_TYPE*/
#define BIT_GPD3_EDGE_TYPE(x)                                               (((x) & 0x3) << 6)
#define BIT_GPD2_EDGE_TYPE(x)                                               (((x) & 0x3) << 4)
#define BIT_GPD1_EDGE_TYPE(x)                                               (((x) & 0x3) << 2)
#define BIT_GPD0_EDGE_TYPE(x)                                               (((x) & 0x3))

/*REG_GPD0_CNT*/
#define BIT_GPD_GLITCH_CNT_MASK                                             0xff
#define BIT_GPD_EDGA_CNT_MASK                                               0xffff
#define BIT_GPD0_101_GLITCH_FILTER_CNT(x)                                   (((x) & 0xff) << 24)
#define BIT_GPD0_010_GLITCH_FILTER_CNT(x)                                   (((x) & 0xff) << 16)
#define BIT_GPD0_EDGE_CNT_MAX(x)                                            (((x) & 0xffff))

/*REG_GPD0_EDGE*/
#define BIT_GPD0_EDGE_CNT(x)                                                (((x) & 0xffff))

/*REG_GPD0_CYCLE_CNT*/
#define BIT_GPD0_CYCLE_CNT(x)                                               (((x) & 0xffffffff))

/*REG_GPD1_CNT*/
#define BIT_GPD1_101_GLITCH_FILTER_CNT(x)                                   (((x) & 0xff) << 24)
#define BIT_GPD1_010_GLITCH_FILTER_CNT(x)                                   (((x) & 0xff) << 16)
#define BIT_GPD1_EDGE_CNT_MAX(x)                                            (((x) & 0xffff))

/*REG_GPD1_EDGE*/
#define BIT_GPD1_EDGE_CNT(x)                                                (((x) & 0xffff))

/*REG_GPD1_CYCLE_CNT*/
#define BIT_GPD1_CYCLE_CNT(x)                                               (((x) & 0xffffffff))

/*REG_GPD2_CNT*/
#define BIT_GPD2_101_GLITCH_FILTER_CNT(x)                                   (((x) & 0xff) << 24)
#define BIT_GPD2_010_GLITCH_FILTER_CNT(x)                                   (((x) & 0xff) << 16)
#define BIT_GPD2_EDGE_CNT_MAX(x)                                            (((x) & 0xffff))

/*REG_GPD2_EDGE*/
#define BIT_GPD2_EDGE_CNT(x)                                                (((x) & 0xffff))

/*REG_GPD2_CYCLE_CNT*/
#define BIT_GPD2_CYCLE_CNT(x)                                               (((x) & 0xffffffff))

/*REG_GPD3_CNT*/
#define BIT_GPD3_101_GLITCH_FILTER_CNT(x)                                   (((x) & 0xff) << 24)
#define BIT_GPD3_010_GLITCH_FILTER_CNT(x)                                   (((x) & 0xff) << 16)
#define BIT_GPD3_EDGE_CNT_MAX(x)                                            (((x) & 0xffff))

/*REG_GPD3_EDGE*/
#define BIT_GPD3_EDGE_CNT(x)                                                (((x) & 0xffff))

/*REG_GPD3_CYCLE_CNT*/
#define BIT_GPD3_CYCLE_CNT(x)                                               (((x) & 0xffffffff))

/*REG_GPD_INT_EN_SET*/
#define BIT_GPD3_INT_EN_SET                                                 BIT(3)
#define BIT_GPD2_INT_EN_SET                                                 BIT(2)
#define BIT_GPD1_INT_EN_SET                                                 BIT(1)
#define BIT_GPD0_INT_EN_SET                                                 BIT(0)

/*REG_GPD_INT_STATUS_CLR_SET*/
#define BIT_GPD3_INT_STATUS_CLR                                             BIT(3)
#define BIT_GPD2_INT_STATUS_CLR                                             BIT(2)
#define BIT_GPD1_INT_STATUS_CLR                                             BIT(1)
#define BIT_GPD0_INT_STATUS_CLR                                             BIT(0)

/*REG_GPD_INT_EN_CLR*/
#define BIT_GPD3_INT_EN_CLR                                                 BIT(3)
#define BIT_GPD2_INT_EN_CLR                                                 BIT(2)
#define BIT_GPD1_INT_EN_CLR                                                 BIT(1)
#define BIT_GPD0_INT_EN_CLR                                                 BIT(0)

/*REG_GPD_INT_STATUS_CLR_CLR*/
#define BIT_GPD3_INT_STATUS_CLR                                             BIT(3)
#define BIT_GPD2_INT_STATUS_CLR                                             BIT(2)
#define BIT_GPD1_INT_STATUS_CLR                                             BIT(1)
#define BIT_GPD0_INT_STATUS_CLR                                             BIT(0)

#endif /*__GPD_RF_H__*/
