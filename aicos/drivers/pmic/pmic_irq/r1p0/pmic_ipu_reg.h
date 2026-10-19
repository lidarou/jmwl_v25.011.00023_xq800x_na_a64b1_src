
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-08-29 15:15:58
 */

#ifndef __PMIC_IPU_H__
#define __PMIC_IPU_H__

#define REG_IPU_INT_REQ_RAW                                                     (0x0)
#define REG_IPU_INT_REQ_ENABLE                                                  (0x4)
#define REG_IPU_INT_REQ_MASK                                                    (0x8)
#define REG_IPU_INT_REQ_POL                                                     (0xc)
#define REG_IPU_INT_REQ_SOFT_SEL                                                (0x10)
#define REG_IPU_INT_REQ_SOFT_SRC                                                (0x14)
#define REG_IPU_IP_INFO                                                         (0xff)
#define REG_IPU_INT_REQ_ENABLE_SET                                              (0x104)
#define REG_IPU_INT_REQ_POL_SET                                                 (0x10c)
#define REG_IPU_INT_REQ_SOFT_SEL_SET                                            (0x110)
#define REG_IPU_INT_REQ_SOFT_SRC_SET                                            (0x114)
#define REG_IPU_INT_REQ_ENABLE_CLR                                              (0x204)
#define REG_IPU_INT_REQ_POL_CLR                                                 (0x20c)
#define REG_IPU_INT_REQ_SOFT_SEL_CLR                                            (0x210)
#define REG_IPU_INT_REQ_SOFT_SRC_CLR                                            (0x214)

/*REG_IPU_INT_REQ_RAW*/
#define BIT_IPU_INT_REQ_RAW(x)                                                  (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_ENABLE*/
#define BIT_IPU_INT_REQ_ENABLE(x)                                               (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_MASK*/
#define BIT_IPU_INT_REQ_MASK(x)                                                 (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_POL*/
#define BIT_IPU_INT_REQ_POL(x)                                                  (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_SOFT_SEL*/
#define BIT_IPU_INT_REQ_SOFT_SEL(x)                                             (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_SOFT_SRC*/
#define BIT_IPU_INT_REQ_SOFT_SRC(x)                                             (((x) & 0xffffffff))

/*REG_IPU_IP_INFO*/
#define BIT_IPU_IP_MAIN_VER(x)                                                  (((x) & 0xff) << 8)
#define BIT_IPU_IP_SUB_VER(x)                                                   (((x) & 0xff))

/*REG_IPU_INT_REQ_ENABLE_SET*/
#define BIT_IPU_INT_REQ_ENABLE(x)                                               (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_POL_SET*/
#define BIT_IPU_INT_REQ_POL(x)                                                  (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_SOFT_SEL_SET*/
#define BIT_IPU_INT_REQ_SOFT_SEL(x)                                             (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_SOFT_SRC_SET*/
#define BIT_IPU_INT_REQ_SOFT_SRC(x)                                             (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_ENABLE_CLR*/
#define BIT_IPU_INT_REQ_ENABLE(x)                                               (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_POL_CLR*/
#define BIT_IPU_INT_REQ_POL(x)                                                  (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_SOFT_SEL_CLR*/
#define BIT_IPU_INT_REQ_SOFT_SEL(x)                                             (((x) & 0xffffffff))

/*REG_IPU_INT_REQ_SOFT_SRC_CLR*/
#define BIT_IPU_INT_REQ_SOFT_SRC(x)                                             (((x) & 0xffffffff))

#endif /*__PMIC_IPU_H__*/

